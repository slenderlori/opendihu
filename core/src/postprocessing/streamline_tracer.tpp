#include "postprocessing/streamline_tracer.h"

#include <algorithm>
#include <petscvec.h>

#include "utility/python_utility.h"

namespace Postprocessing
{

template<typename DiscretizableInTimeType>
StreamlineTracer<DiscretizableInTimeType>::
StreamlineTracer(DihuContext context) :
  context_(context["StreamlineTracer"]), problem_(context_), data_(context_)
{
  LOG(TRACE) << "StreamlineTracer::StreamlineTracer()";

  specificSettings_ = context_.getPythonConfig();
  VLOG(2) << "in StreamlineTracer(), specificSettings_: " << specificSettings_;
  outputWriterManager_.initialize(context_, specificSettings_);

  lineStepWidth_ = PythonUtility::getOptionDouble(specificSettings_, "lineStepWidth", 1e-2, PythonUtility::Positive);
  targetElementLength_ = PythonUtility::getOptionDouble(specificSettings_, "targetElementLength", 0.0, PythonUtility::Positive);
  targetLength_ = PythonUtility::getOptionDouble(specificSettings_, "targetLength", 0.0, PythonUtility::Positive);
  discardRelativeLength_ = PythonUtility::getOptionDouble(specificSettings_, "discardRelativeLength", 0.0, PythonUtility::Positive);
  maxNIterations_ = PythonUtility::getOptionInt(specificSettings_, "maxIterations", 100000, PythonUtility::Positive);
  useGradientField_ = PythonUtility::getOptionBool(specificSettings_, "useGradientField", false);
  csvFilename_ = PythonUtility::getOptionString(specificSettings_, "csvFilename", "");
  csvFilenameBeforePostprocessing_ = PythonUtility::getOptionString(specificSettings_, "csvFilenameBeforePostprocessing", "");
  
  // get the first seed position from the list
  PyObject *pySeedPositions = PythonUtility::getOptionListBegin<PyObject *>(specificSettings_, "seedPoints");

  // loop over other entries of list
  for (;
      !PythonUtility::getOptionListEnd(specificSettings_, "seedPoints");
      PythonUtility::getOptionListNext<PyObject *>(specificSettings_, "seedPoints", pySeedPositions))
  {
    Vec3 seedPosition = PythonUtility::convertFromPython<Vec3>::get(pySeedPositions);
    seedPositions_.push_back(seedPosition);
  }
}

template<typename DiscretizableInTimeType>
void StreamlineTracer<DiscretizableInTimeType>::
initialize()
{
  LOG(TRACE) << "StreamlineTracer::initialize";

  // initialize the problem
  problem_.initialize();

  // initialize streamline tracer data object
  data_.setBaseData(std::make_shared<typename DiscretizableInTimeType::Data>(problem_.data()));
  data_.initialize();
}

template<typename DiscretizableInTimeType>
void StreamlineTracer<DiscretizableInTimeType>::
run()
{
  initialize();

  // call the method of the underlying problem
  problem_.run();

  // do the tracing
  traceStreamlines();

  // output
  outputWriterManager_.writeOutput(data_);
}

template<typename DiscretizableInTimeType>
void StreamlineTracer<DiscretizableInTimeType>::
traceStreamline(element_no_t initialElementNo, std::array<double,(unsigned long int)3> xi, Vec3 startingPoint, double direction, std::vector<Vec3> &points)
{
  const int D = DiscretizableInTimeType::FunctionSpace::dim();
  
  const int nDofsPerElement = DiscretizableInTimeType::FunctionSpace::nDofsPerElement();
  
  Vec3 currentPoint = startingPoint;
  element_no_t elementNo = initialElementNo;
  
    std::array<Vec3,nDofsPerElement> elementalGradientValues;
    std::array<double,nDofsPerElement> elementalSolutionValues;
    std::array<Vec3,nDofsPerElement> geometryValues;

   // get gradient values for element
   // There are 2 implementations of streamline tracing. 
   // The first one (useGradientField_) uses a precomputed gradient field that is interpolated linearly and the second uses the gradient directly from the Laplace solution field. 
   // The first one seems more stable, because the gradient is zero and the position of the boundary conditions and should be used with a linear discretization of the potential field. 
   // The second one is more accurate.
   if (useGradientField_)
   {
     // use the precomputed gradient field
     data_.gradient()->getElementValues(elementNo, elementalGradientValues);
   }
   else 
   {
     // get the local gradient value at the current position
     problem_.data().solution()->getElementValues(elementNo, elementalSolutionValues);

     // get geometry field (which are the node positions for Lagrange basis and node positions and derivatives for Hermite)
     problem_.data().functionSpace()->getElementGeometry(elementNo, geometryValues);
   }
   
   VLOG(2) << "streamline starts in element " << elementNo;
   
   // loop over length of streamline, avoid loops by limiting the number of iterations
   for(int iterationNo = 0; iterationNo <= maxNIterations_; iterationNo++)
   {
     if (iterationNo == maxNIterations_)
     {
       LOG(WARNING) << "streamline reached maximum number of iterations (" << maxNIterations_ << ")";
       points.clear();
       break;
     }
    
     // check if element_no is still valid
     if (!problem_.data().functionSpace()->pointIsInElement(currentPoint, elementNo, xi))
     {
       bool positionFound = problem_.data().functionSpace()->findPosition(currentPoint, elementNo, xi);

       // if no position was found, the streamline exits the domain
       if (!positionFound)
       {
         VLOG(2) << "streamline ends at iteration " << iterationNo << " because " << currentPoint << " is outside of domain";
         break;
       }
           
       // get values for element that are later needed to compute the gradient
       if (useGradientField_)      
       {
         data_.gradient()->getElementValues(elementNo, elementalGradientValues);
       }
       else 
       {
         problem_.data().solution()->getElementValues(elementNo, elementalSolutionValues);
           
         // get geometry field (which are the node positions for Lagrange basis and node positions and derivatives for Hermite)
         problem_.data().functionSpace()->getElementGeometry(elementNo, geometryValues);
       }
       
       VLOG(2) << "streamline enters element " << elementNo;
     }

     // get value of gradient
     Vec3 gradient;
     if (useGradientField_)
     {       
       gradient = problem_.data().functionSpace()->template interpolateValueInElement<3>(elementalGradientValues, xi);
       VLOG(2) << "use gradient field";
     }
     else 
     {
       // compute the gradient value in the current value
       Tensor2<D> inverseJacobian = problem_.data().functionSpace()->getInverseJacobian(geometryValues, elementNo, xi);
       gradient = problem_.data().functionSpace()->interpolateGradientInElement(elementalSolutionValues, inverseJacobian, xi);
       
       VLOG(2) << "use direct gradient";
     }
     
     // integrate streamline
     VLOG(2) << "  integrate from " << currentPoint << ", gradient: " << gradient << ", gradient normalized: " << MathUtility::normalized<3>(gradient) << ", lineStepWidth: " << lineStepWidth_;
     currentPoint = currentPoint + MathUtility::normalized<3>(gradient)*lineStepWidth_*direction;
     
     VLOG(2) << "              to " << currentPoint;
     
     points.push_back(currentPoint);
   }
}


template<typename DiscretizableInTimeType>
void StreamlineTracer<DiscretizableInTimeType>::
traceStreamlines()
{
  LOG(TRACE) << "traceStreamlines";

  // compute a gradient field from the solution
  problem_.data().solution()->computeGradientField(data_.gradient());
 
  std::array<double,(unsigned long int)3> xi;
  const int nSeedPoints = seedPositions_.size();
  //const int nSeedPoints = 1;
  std::vector<std::vector<Vec3>> streamlines(nSeedPoints);

  LOG(DEBUG) << "trace streamline, seedPositions: " << seedPositions_;
  
  // loop over seed points
  //#pragma omp parallel for shared(streamlines)
  for (int seedPointNo = 0; seedPointNo < nSeedPoints; seedPointNo++)
  {
    // get starting point
    Vec3 startingPoint = seedPositions_[seedPointNo];
    //streamlines[seedPointNo].push_back(startingPoint);

    element_no_t initialElementNo = 0;
    
    // find out initial element no and xi value where the current Point lies
    bool positionFound = problem_.data().functionSpace()->findPosition(startingPoint, initialElementNo, xi);
    if (!positionFound)
    {
      LOG(ERROR) << "Seed point " << startingPoint << " is outside of domain.";
      continue;
    }
    
    // trace streamline forwards
    std::vector<Vec3> forwardPoints;
    traceStreamline(initialElementNo, xi, startingPoint, 1.0, forwardPoints);
    
    // trace streamline backwards
    std::vector<Vec3> backwardPoints;
    traceStreamline(initialElementNo, xi, startingPoint, -1.0, backwardPoints);
  
    // copy collected points to result vector 
    streamlines[seedPointNo].insert(streamlines[seedPointNo].begin(), backwardPoints.rbegin(), backwardPoints.rend());
    streamlines[seedPointNo].insert(streamlines[seedPointNo].end(), startingPoint);
    streamlines[seedPointNo].insert(streamlines[seedPointNo].end(), forwardPoints.begin(), forwardPoints.end());
    
    LOG(DEBUG) << " seed point " << seedPointNo << ", " << streamlines[seedPointNo].size() << " points";
  }
  
  // create 1D meshes of streamline from collected node positions
  if (!csvFilenameBeforePostprocessing_.empty())
  {
    std::ofstream file(csvFilenameBeforePostprocessing_, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open())
      LOG(WARNING) << "Could not open \"" << csvFilenameBeforePostprocessing_ << "\" for writing";
    
    for (int streamlineNo = 0; streamlineNo != streamlines.size(); streamlineNo++)
    {
      for (std::vector<Vec3>::const_iterator iter = streamlines[streamlineNo].begin(); iter != streamlines[streamlineNo].end(); iter++)
      {
        Vec3 point = *iter;
        file << point[0] << ";" << point[1] << ";" << point[2] << ";";
      }
      file << "\n";
    }
    file.close();
    LOG(INFO) << "File \"" << csvFilenameBeforePostprocessing_ << "\" written.";
  }
  
  // coarsen streamlines and drop too small streamlines
  postprocessStreamlines(streamlines);
 
  LOG(DEBUG) << "number streamlines after postprocessStreamlines: " << streamlines.size();
  
  // create 1D meshes of streamline from collected node positions
  if (!csvFilename_.empty())
  {
    std::ofstream file(csvFilename_, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open())
      LOG(WARNING) << "Could not open \"" << csvFilename_ << "\" for writing";
    
    for (int streamlineNo = 0; streamlineNo != streamlines.size(); streamlineNo++)
    {
      for (std::vector<Vec3>::const_iterator iter = streamlines[streamlineNo].begin(); iter != streamlines[streamlineNo].end(); iter++)
      {
        Vec3 point = *iter;
        file << point[0] << ";" << point[1] << ";" << point[2] << ";";
      }
      file << "\n";
    }
    file.close();
    LOG(INFO) << "File \"" << csvFilename_ << "\" written.";
  }
  
  // create new meshes, one for each streamline 
  for (int streamlineNo = 0; streamlineNo != streamlines.size(); streamlineNo++)
  {
    LOG(DEBUG) << "seed point " << streamlineNo << ", number node positions: " << streamlines[streamlineNo].size();
    this->data_.createFibreMesh(streamlines[streamlineNo]);
  }
}

template<typename DiscretizableInTimeType>
void StreamlineTracer<DiscretizableInTimeType>::
postprocessStreamlines(std::vector<std::vector<Vec3>> &streamlines)
{
  std::vector<double> lengths(streamlines.size());
 
  // compute length of each streamline 
  int i = 0;
  // loop over streamlines
  for (std::vector<std::vector<Vec3>>::iterator streamlinesIter = streamlines.begin(); streamlinesIter != streamlines.end(); streamlinesIter++, i++)
  {
    lengths[i] = 0.0;
    
    Vec3 lastPoint;
    bool firstPoint = true;
    int pointNo = 0;
    
    // loop over points of streamline
    for (std::vector<Vec3>::iterator pointsIter = streamlinesIter->begin(); pointsIter != streamlinesIter->end(); pointsIter++, pointNo++)
    {
      if (!firstPoint)
      {
        lengths[i] += MathUtility::distance<3>(*pointsIter, lastPoint);
      }
      firstPoint = false;
      lastPoint = *pointsIter;
    }
  }
  
  LOG(DEBUG) << " lengths of streamlines: " << lengths;
  
  // sort length
  std::vector<double> lengthsSorted(lengths);
  std::sort(lengthsSorted.begin(), lengthsSorted.end());
  
  // get median 
  double medianLength = lengthsSorted[lengthsSorted.size()/2];
  double maximumLength = lengthsSorted[lengthsSorted.size()-1];
  LOG(INFO) << "The median length of the streamlines is " << medianLength 
    << ", the maximum length of the " << lengthsSorted.size() << " streamlines is " << maximumLength << ".";
    
  if (discardRelativeLength_ != 0.0)
  {
   
    // clear streamlines that are shorter than discardRelativeLength_
    // loop over streamlines
    i = 0;
    for (std::vector<std::vector<Vec3>>::iterator streamlinesIter = streamlines.begin(); streamlinesIter != streamlines.end(); streamlinesIter++, i++)
    { 
      if (lengths[i] < discardRelativeLength_*medianLength) 
      {
        LOG(INFO) << "Discarding streamline no. " << i << " with length " << lengths[i] << " (Threshold " << discardRelativeLength_*medianLength << ").";
        streamlinesIter->clear();
      }
    }
    
    auto lastValidStreamline = std::remove_if(streamlines.begin(), streamlines.end(), 
                                              [](const std::vector<Vec3> &a)-> bool{return a.empty();});
    
    // remove previously cleared streamlines
    streamlines.erase(
       lastValidStreamline, 
       streamlines.end()
    );
  }
  
  // compute scale factor that scales streamlines to targetLength
  double scalingFactor = 1.0;
  if (targetLength_ != 0)
  {
    scalingFactor = targetLength_/maximumLength;
  }
  
  // resample streamlines
  if (targetElementLength_ != 0.0 && targetElementLength_ != lineStepWidth_)
  {
    // loop over streamlines
    for (int i = 0; i < streamlines.size(); i++)
    {
      std::vector<Vec3> &currentStreamline = streamlines[i];
      
      if (currentStreamline.empty())
      {
        LOG(DEBUG) << "Streamline is empty";
      }
      else 
      {
        std::vector<Vec3> newStreamline;
        int presumedLength = int(currentStreamline.size()*targetElementLength_/lineStepWidth_+10);
        newStreamline.reserve(presumedLength);
        
        VLOG(1) << "streamline no " << i << ", reserve length " << presumedLength;
        VLOG(1) << "targetElementLength_: " << targetElementLength_ << ", scalingFactor: " << scalingFactor;
        
        Vec3 lastPoint = currentStreamline.front()*scalingFactor;
        Vec3 previousStreamlinePoint = lastPoint;  // last point that was inserted into the new streamline
        // use starting point of streamline
        newStreamline.push_back(lastPoint);
        double length = 0.0;
        bool firstPoint = true;
        
        // loop over points of streamline
        for (std::vector<Vec3>::iterator pointsIter = currentStreamline.begin(); pointsIter != currentStreamline.end(); pointsIter++)
        {
          if (!firstPoint)
          {
            Vec3 currentPoint = (*pointsIter)*scalingFactor;
            // sum up length since last element started
            length += MathUtility::length<3>(currentPoint - lastPoint);
            
            VLOG(1) << "old streamline interval " << lastPoint << " - " << currentPoint << ", new lentgh: " << length << " (targetElementLength=" << targetElementLength_ << ")";
            
            if (length > targetElementLength_)
            {
              double alpha = targetElementLength_/length;
              Vec3 point = (1. - alpha) * previousStreamlinePoint + alpha * currentPoint;
             
              VLOG(1) << "  length is too big, alpha=" << alpha << ", take intermediate point " << point
                << ", distance to previous point " << previousStreamlinePoint << ": " << MathUtility::length<3>(point - previousStreamlinePoint);
              
              newStreamline.push_back(point);
              
              previousStreamlinePoint = point;
              lastPoint = previousStreamlinePoint;
              length = 0.0;
            }
            else 
            {
              lastPoint = currentPoint;
            }
            
          }
          firstPoint = false;
        }
        LOG(DEBUG) << "Scaled streamline by factor " << scalingFactor << ", resampled from lineStepWidth " << lineStepWidth_ << " to targetElementLength " << targetElementLength_ 
          << ", now it has " << newStreamline.size() << " points, length " << lengths[i]*scalingFactor;
        streamlines[i] = newStreamline;
      }
    }
  }
    
  LOG(DEBUG) << "Number of streamlines after resampling: " << streamlines.size();
}


};

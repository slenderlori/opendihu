#include "model_order_reduction/time_stepping_scheme_ode_reduced.h"

#include <Python.h>
#include "utility/python_utility.h"
#include "time_stepping_scheme/time_stepping_scheme.h"

namespace ModelOrderReduction
{
template<typename TimeSteppingType>
TimeSteppingSchemeOdeReduced<TimeSteppingType>::
TimeSteppingSchemeOdeReduced(DihuContext context):
MORBase<typename TimeSteppingType::DiscretizableInTimeType::FunctionSpace>(context["ModelOrderReduction"]), TimeSteppingScheme(context["ModelOrderReduction"]),
timestepping_(context_["ModelOrderReduction"]), solutionVectorMapping_(SolutionVectorMapping()), initialized_(false)
{
  PyObject *topLevelSettings = this->context_.getPythonConfig();
  this->specificSettings_ = PythonUtility::getOptionPyObject(topLevelSettings, "ModelOrderReduction");
  nReducedBases_ = PythonUtility::getOptionInt(specificSettings_, "nReducedBases", 10, PythonUtility::Positive); 
}

template<typename TimesteppingType>
FieldVariable::FieldVariable<FunctionSpace::Generic,1> &TimeSteppingSchemeOdeReduced<TimesteppingType>::
solution()
{  
  return this->data_->redSolution();
}

template<typename TimeSteppingType>
SolutionVectorMapping &TimeSteppingSchemeOdeReduced<TimeSteppingType>::
solutionVectorMapping()
{
  return solutionVectorMapping_;
}

template<typename TimeSteppingType>
void TimeSteppingSchemeOdeReduced<TimeSteppingType>::
initialize()
{
  if (initialized_)
    return;
  
  LOG(TRACE) << "TimeSteppingSchemeOdeReduced::initialize()";
  
  this->timestepping_.initialize();
  
  std::array<element_no_t, 1> nElements({nReducedBases_ - 1});
  std::array<double, 1> physicalExtent({0.0});
  
  // create the functionspace for the reduced order
  std::shared_ptr<FunctionSpace::Generic> functionSpaceRed = context_.meshManager()->createFunctionSpace<>("functionSpaceReduced", nElements, physicalExtent);
  this->data_->setFunctionSpace(functionSpaceRed);
  this->data_->setFullFunctionspace(timestepping_.discretizableInTime_.data().functionspace());
  this->data_->initialize();
  
  MORBase::initialize();
}

template<typename TimeSteppingType>
void TimeSteppingSchemeOdeReduced<TimeSteppingType>::
run()
{
  // initialize
  this->initialize();
  
  // do simulations
  this->advanceTimeSpan();    
}
  
template<typename TimeSteppingType>
void TimeSteppingSchemeOdeReduced<TimeSteppingType>::
reset()
{
  TimeSteppingScheme::reset();
  timestepping_.reset();
  
  initialized_ = false;
}
  
  template<typename TimeSteppingType>
  bool TimeSteppingSchemeOdeReduced<TimeSteppingType>::
  knowsMeshType()
  {
    return false;
  }
  
} //namespace
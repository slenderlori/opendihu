#include "mesh/mesh_manager.h"

#include "function_space/function_space.h"
#include "mesh/structured_regular_fixed.h"
#include "mesh/unstructured_deformable.h"

namespace Mesh
{

Manager::Manager(PyObject *specificSettings) :
  partitionManager_(nullptr), specificSettings_(specificSettings), numberAnonymousMeshes_(0)
{
  LOG(TRACE) << "MeshManager constructor";
  storePreconfiguredMeshes();
}

void Manager::setPartitionManager(std::shared_ptr<Partition::Manager> partitionManager)
{
  partitionManager_ = partitionManager;
}

void Manager::storePreconfiguredMeshes()
{
  LOG(TRACE) << "MeshManager::storePreconfiguredMeshes";
  if (specificSettings_)
  {
    std::string keyString("Meshes");
    if (PythonUtility::hasKey(specificSettings_, "Meshes"))
    {

      std::pair<std::string, PyObject *> dictItem
        = PythonUtility::getOptionDictBegin<std::string, PyObject *>(specificSettings_, keyString);

      for (; !PythonUtility::getOptionDictEnd(specificSettings_, keyString);
          PythonUtility::getOptionDictNext<std::string, PyObject *>(specificSettings_, keyString, dictItem))
      {
        std::string key = dictItem.first;
        PyObject *value = dictItem.second;

        if (value == NULL)
        {
          LOG(WARNING) << "Could not extract dict for Mesh \"" <<key<< "\".";
        }
        else if(!PyDict_Check(value))
        {
          LOG(WARNING) << "Value for mesh with name \"" <<key<< "\" should be a dict.";
        }
        else
        {
          LOG(DEBUG) << "store mesh configuration with key \"" <<key<< "\".";
          meshConfiguration_[key] = value;
        }
      }
    }
    else
    {
      LOG(INFO) << "You have specified the mesh in-line and not under the extra key \"Meshes\". You could do so,"
        " by defining \"Meshes\": {\"<your custom mesh name>\": {<your mesh parameters>}} at the beginning of the"
        " config and \"meshName\": \"<your custom mesh name>\" where you currently have specified the mesh parameters."
        " This is required if you want to use the same mesh for multiple objects.";
    }
  }
}

std::shared_ptr<FieldVariable::FieldVariable<FunctionSpace::Generic,1>> Manager::
createGenericFieldVariable(int nEntries, std::string name)
{
  assert(nEntries > 1);

  // create generic field variable

  // constructor is declared in function_space/06_function_space_dofs_nodes.h
  // FunctionSpaceDofsNodes(std::shared_ptr<Partition::Manager> partitionManager, std::array<element_no_t, D> nElements, std::array<double, D> physicalExtent);

  std::array<element_no_t, 1> nElements({nEntries - 1});
  std::array<double, 1> physicalExtent({0.0});
  std::stringstream meshName;
  meshName << "meshForFieldVariable" << name;
  std::shared_ptr<Mesh> mesh = createFunctionSpace<FunctionSpace::Generic>(meshName.str(), nElements, physicalExtent);

  LOG(DEBUG) << "create generic field variable with " << nEntries << " entries.";
  std::shared_ptr<FunctionSpace::Generic> functionSpace = std::static_pointer_cast<FunctionSpace::Generic>(mesh);

  // createFieldVariable is declared in function_space/09_function_space_field_variable.h
  //template <int nComponents>
  //std::shared_ptr<FieldVariable::FieldVariable<FunctionSpace<MeshType,BasisFunctionType>,nComponents>> createFieldVariable(std::string name);
  return functionSpace->template createFieldVariable<1>(name);
}

bool Manager::hasFunctionSpace(std::string meshName)
{
  return functionSpaces_.find(meshName) != functionSpaces_.end();
}

};  // namespace

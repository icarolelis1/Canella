#ifndef CANELLA_MATERIALMANAGER_H
#define CANELLA_MATERIALMANAGER_H
#include "Render/Render.h"
#include <vector>
#include <unordered_map>
#include <string>
namespace Canella {

   struct MaterialCollection {
       std::vector<MaterialData> collection;
       std::unordered_map<std::string,bool> material_loaded_record;
   };

   bool is_material_loaded(std::string& material_name,MaterialCollection& collection);
}

#endif //CANELLA_MATERIALMANAGER_H

//
// Created by icaro on 18/09/2023.
//
#include "MaterialManager/MaterialManager.h"

bool Canella::is_material_loaded( std::string& material_name, Canella::MaterialCollection &collection ) {
    {
        return collection.material_loaded_record.find( material_name ) != collection.material_loaded_record.end();
    }
}

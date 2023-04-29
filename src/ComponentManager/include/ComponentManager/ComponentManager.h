//#include <memory>
//#ifndef COMPONENT_SYSTEM
//#include "rEntity/Entity.h"
//#include <vector>
//#include <unordered_map>
//#include <memory>
//namespace Canella
//{
//    /**
//     * This file is insipired by Wicked Engine Component System
//     * https://wickedengine.net/2019/09/29/entity-component-system/
//     */
//
//
//    class ComponentManagerInterface
//    {
//		virtual CanellaRefactor::Entity getEntity(size_t index) const = 0;
//        ~ComponentManagerInterface() = default;
//    };
//    
//    template <typename Component_>
//    class ComponentManager :public ComponentManagerInterface
//    {
//    private:
//        std::unordered_map<CanellaRefactor::Entity,uint32_t> map;
//        std::vector<Component_> components;
//        std::vector<CanellaRefactor::Entity> entities;
//
//    public:
//    
//        Component_ attachToEntity(uint32_t entity_id)
//        {
//            if (map.find(entity_id) == map.end())
//                map[entity_id] = components.size();
//        }
//
//        void MoveItem(size_t index_from, size_t index_to)
//        {
//            assert(index_from < GetCount());
//            assert(index_to < GetCount());
//            if (index_from == index_to)
//            {
//                return;
//            }
//
//            // Save the moved component and entity:
//            Component component = std::move(components[index_from]);
//            Entity entity = entities[index_from];
//
//            // Every other entity-component that's in the way gets moved by one and lut is kept updated:
//            const int direction = index_from < index_to ? 1 : -1;
//            for (size_t i = index_from; i != index_to; i += direction)
//            {
//                const size_t next = i + direction;
//                components[i] = std::move(components[next]);
//                entities[i] = entities[next];
//                lookup[entities[i]] = i;
//            }
//
//            // Saved entity-component moved to the required position:
//            components[index_to] = std::move(component);
//            entities[index_to] = entity;
//            lookup[entity] = index_to;
//        }
//
//        Component_ createComponent(CanellaRefactor::Entity entity){}
//
//        Component_*  getComponent(CanellaRefactor::Entity entity)
//        {
//            auto it = map.find(entity);
//            if(it!= map.end())
//                return &components[it->second];
//            return nullptr;
//        }
//
//        Component_& create(CanellaRefactor::Entity entity)
//        {
//            assert(entity.id != -1);
//            assert(map.find(entity) == map.end());
//            assert(entities.size() == components.size());
//            map[entity] = components.size();
//            entities.push_back(entity);
//            components.push_back(Component_());
//            entities.push_back(entity);
//
//            return components.back();
//        }
//
//        inline CanellaRefactor::Entity getEntity(size_t index) const
//        {
//            return entities[index];
//        }
//        
//        Component_& operator[](size_t index) {return components[index];}
//        
//        bool Contains(CanellaRefactor::Entity entity){
//            return map.find(entity)!=map.end();
//        }
//
//        void remove(CanellaRefactor::Entity entity)
//        {
//            auto it = map.find(entity);
//            if(it == map.end())return;
//            const size_t index = it->second;
//            if (index < components.size() - 1)
//            {
//                components[index] = std::move(components.back());
//                entities[index] = entities.back();
//                map[entities[index]] = index;
//            }
//            
//            components.pop_back();
//            entities.pop_back();
//            map.erase(entity);
//        }
//        inline size_t GetCount()
//        {
//            return components.size();
//        }
//        
//    };
//
//    class ComponentLibrary
//    {
//    public:
//        struct LibraryEntry
//        {
//            std::unique_ptr<ComponentManagerInterface> component_manager;
//        };
//        std::unordered_map<std::string, LibraryEntry> componentLibrary;
//        template <typename  T>
//        inline ComponentManager<T>& registerComponent(std::string identifier){
//            componentLibrary[identifier] = std::make_unique<ComponentManager<T>>();
//            return static_cast<ComponentManager<T>&>(*componentLibrary[name].component_manager);
//        }
//            
//    };
//    
//} // Canella
//#endif
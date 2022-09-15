/*
    This file is part of darts – the Dartmouth Academic Ray Tracing Skeleton.

    Copyright (c) 2017-2022 by Wojciech Jarosz
*/
#pragma once

#include <darts/json.h>

/// Abstract factory used to construct objects by name.
/// \ingroup Parser
template <typename Object, typename... Args>
class Factory
{
public:
    using Constructor = std::function<Object(Args &&...args)>;

    /**
        Construct an object from the class of the given \p name and pass \p args to the registered constructor.

        \param name
            An internal name that is associated with this class. For darts scenes, this is the
            'type' field found in the scene json file.

        \param args
            Variadic list of arguments to forward to the registered constructor.
     */
    static Object create_instance(const std::string &name, Args &&...args)
    {
        if (type_registry().find(name) == type_registry().end())
            throw DartsException("Cannot find a constructor for type '{}' in the factory.", name);
        return type_registry().at(name)(std::forward<Args>(args)...);
    }

    /**
        Register an object constructor with the object factory

        This function is called by the macro #DARTS_REGISTER_CLASS_IN_FACTORY

        \param name
            An internal name that is associated with this class. For darts scenes, this is the
            'type' field found in the scene json file.

        \param constructor
            A function that is able to call the constructor of the class.
       */
    static void register_type(const std::string &name, const Constructor constructor)
    {
        if (type_registry().find(name) != type_registry().end())
            throw DartsException("A constructor for type '{}' has already been registered in this factory.", name);

        type_registry().insert(std::make_pair(name, constructor));
    }

    /// Return a set of all classes registered with this factory (useful for debugging).
    static std::set<string> registered_types()
    {
        const auto       r = type_registry();
        std::set<string> keys;
        for (const auto mi : r)
            keys.insert(mi.first);
        return keys;
    }

protected:
    /// Singleton pattern that prevents "static initialization order fiasco".
    static std::map<std::string, Constructor> &type_registry()
    {
        static std::map<std::string, Constructor> registry;
        return registry;
    }
};

/**
    A specialization of the #Factory for darts objects read from json

    Additionally, DartsFactory allows storing previously parsed/created instances into a registry with
    #register_instance() and retrieving them with their name using #find().

    \warning If you want to use DartsFactory<T>::find() with more types T, you need to specialize it

    \tparam T This factory will create objects of type shared_ptr<T>.

    \ingroup Parser
*/
template <typename T>
class DartsFactory : public Factory<shared_ptr<T>, const nlohmann::json &>
{
public:
    using SharedT     = shared_ptr<T>; ///< Shared pointer to the created type #T
    using BaseFactory = Factory<SharedT, const nlohmann::json &>;

    /// Allows creating a shared_ptr to a darts object, deducing the type from a "type" field in \p j
    static SharedT create(const nlohmann::json &j);

    /**
        Find a previously parsed/created #SharedT, or create a new one.

        Return a #SharedT pointer by parsing a JSON specification. If \p j is a string \p key: "name", then try to find
        a previously parsed #SharedT with name "name". If \p j is a #json object \p key: {}, then create a new #SharedT
        with the specified parameters by calling #create().

        \param jp           The json object for the parent
        \param key          Look for this json field in \p jp
        \return #SharedT    The newly created or retrieved object instance.
     */
    static SharedT find(const nlohmann::json &jp, const std::string &key)
    {
        auto it = jp.find(key);
        if (it == jp.end())
            // return nullptr;
            throw DartsException("Cannot find key '{}' here:\n{}", key, jp.dump(4));

        auto j = it.value();
        if (j.is_string())
        {
            string name = j.get<string>();
            // find a pre-declared SharedT
            auto i = instance_registry().find(name);
            if (i != instance_registry().end())
                return i->second;
            else
                throw DartsException("Cannot find an object with name '{}' here:\n{}", name, jp.dump(4));
        }
        else if (j.is_object())
        {
            // create a new SharedT instance
            return create(j);
        }
        else
            throw DartsException("Type mismatch: Expecting either a {} definition or {} name here:\n{}", key, key,
                                 jp.dump(4));
    }

    /**
        Find a previously parsed/created #SharedT, or create a new one using the default key for this type of object.

        \warning This function overload needs to be specialized for new types T.

        \overload

        \param jp           The json object for the parent
        \return #SharedT    The newly created or retrieved object instance.
     */
    static SharedT find(const nlohmann::json &jp);

    /// Associate and store a shared_ptr to an object instance \p o with the name \p name
    static void register_instance(const std::string &name, SharedT o)
    {
        instance_registry()[name] = o;
    }

protected:
    /// Global map of #SharedT instances that have been create/parsed
    static std::map<std::string, SharedT> &instance_registry()
    {
        static std::map<std::string, SharedT> registry;
        return registry;
    }
};

template <>
inline shared_ptr<Material> DartsFactory<Material>::find(const nlohmann::json &jp)
{
    return find(jp, "material");
}


template <typename T>
shared_ptr<T> DartsFactory<T>::create(const json &j)
{
    if (!j.contains("type") || !j["type"].is_string())
        throw DartsException("Missing 'type' field in:\n{}", j.dump(4));

    auto type = j.at("type").get<string>();

    try
    {
        return BaseFactory::create_instance(type, j);
    }
    catch (const std::exception &e)
    {
        throw DartsException("Cannot create a '{}' here:\n{}.\n\t{}", type, j.dump(4), e.what());
    }
}


/**
    Macro for registering an object constructor with a #DartsFactory

    \param  T       The object type to register
    \param  cls     The name of the class to create instances of
    \param  name    Associate the keyword `type: "name"` for creating this type of object from a json object

    \ingroup Parser
*/
#define DARTS_REGISTER_CLASS_IN_FACTORY(T, cls, name)                                                                  \
    static struct cls##_FACTORY_HELPER                                                                                 \
    {                                                                                                                  \
        cls##_FACTORY_HELPER()                                                                                         \
        {                                                                                                              \
            DartsFactory<T>::register_type(name, [](const nlohmann::json &j) { return make_shared<cls>(j); });         \
        }                                                                                                              \
    } cls##__FACTORY_HELPER_INSTANCE;

/**
    \file
    \brief Implementation of the abstract factory
*/

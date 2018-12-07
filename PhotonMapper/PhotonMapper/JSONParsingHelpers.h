#pragma once

#include <string>
#include  "picojson.h"

static bool ParseBooleanProperty(bool *ret, std::string *err, const picojson::object &o, const std::string &property, bool required) 
{
    picojson::object::const_iterator it = o.find(property);
    if (it == o.end()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is missing.\n";
            }
        }
        return false;
    }

    if (!it->second.is<bool>()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is not a bool type.\n";
            }
        }
        return false;
    }

    if (ret) 
    {
        (*ret) = it->second.get<bool>();
    }

    return true;
}

bool ParseNumberProperty(double *ret, std::string *err, const picojson::object &o, const std::string &property, bool required) 
{
    picojson::object::const_iterator it = o.find(property);
    if (it == o.end()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is missing.\n";
            }
        }
        return false;
    }

    if (!it->second.is<double>()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is not a number type.\n";
            }
        }
        return false;
    }

    if (ret) 
    {
        (*ret) = it->second.get<double>();
    }

    return true;
}

static bool ParseNumberArrayProperty(std::vector<double> *ret, std::string *err, const picojson::object &o, const std::string &property, bool required) 
{
    picojson::object::const_iterator it = o.find(property);
    if (it == o.end()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is missing.\n";
            }
        }
        return false;
    }

    if (!it->second.is<picojson::array>()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is not an array.\n";
            }
        }
        return false;
    }

    ret->clear();
    const picojson::array &arr = it->second.get<picojson::array>();
    for (size_t i = 0; i < arr.size(); i++) 
    {
        if (!arr[i].is<double>()) 
        {
            if (required) 
            {
                if (err) 
                {
                    (*err) += "'" + property + "' property is not a number.\n";
                }
            }
            return false;
        }
        ret->push_back(arr[i].get<double>());
    }
    return true;
}

bool ParseStringProperty(std::string *ret, std::string *err, const picojson::object &o, const std::string &property, bool required) 
{
    picojson::object::const_iterator it = o.find(property);
    if (it == o.end()) {
        if (required) {
            if (err) {
                (*err) += "'" + property + "' property is missing.\n";
            }
        }
        return false;
    }

    if (!it->second.is<std::string>()) 
    {
        if (required) 
        {
            if (err) 
            {
                (*err) += "'" + property + "' property is not a string type.\n";
            }
        }
        return false;
    }
    if (ret) 
    {
        (*ret) = it->second.get<std::string>();
    }
    return true;
}

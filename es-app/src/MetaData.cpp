//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  MetaData.cpp
//
//  Static data for default metadata values as well as functions
//  to read and write metadata from the gamelist files.
//

#include "MetaData.h"

#include "Log.h"
#include "utils/FileSystemUtil.h"

#include <pugixml.hpp>

// clang-format off
// The statistic entries must be placed at the bottom or otherwise there will be problems with
// saving the values in GuiMetaDataEd.
MetaDataDecl gameDecls[] = {
// key,                 type,                     default,                      statistic,          name in GuiMetaDataEd,                    prompt in GuiMetaDataEd,                        shouldScrape
{"name",          MD_STRING,           "",                false,     "name",                       "enter name",                       true,    {}},
{"sortname",      MD_STRING,           "",                false,     "sortname",                   "enter sortname",                   false,   {}},
{"desc",          MD_MULTILINE_STRING, "",                false,     "description",                "enter description",                true,    {}},
{"rating",        MD_RATING,           "0",               false,     "rating",                     "enter rating",                     true,    {}},
{"releasedate",   MD_DATE,             "19700101T010000", false,     "release date",               "enter release date",               true,    {}},
{"developer",     MD_STRING,           "unknown",         false,     "developer",                  "enter developer",                  true,    {}},
{"publisher",     MD_STRING,           "unknown",         false,     "publisher",                  "enter publisher",                  true,    {}},
{"genre",         MD_STRING,           "unknown",         false,     "genre",                      "enter genre",                      true,    {}},
{"players",       MD_STRING,           "unknown",         false,     "players",                    "enter number of players",          true,    {}},
{"favorite",      MD_BOOL,             "false",           false,     "favorite",                   "enter favorite off/on",            false,   {}},
{"completed",     MD_BOOL,             "false",           false,     "completed",                  "enter completed off/on",           false,   {}},
{"kidgame",       MD_BOOL,             "false",           false,     "kidgame",                    "enter kidgame off/on",             false,   {}},
{"hidden",        MD_BOOL,             "false",           false,     "hidden",                     "enter hidden off/on",              false,   {}},
{"broken",        MD_BOOL,             "false",           false,     "broken/not working",         "enter broken off/on",              false,   {}},
{"nogamecount",   MD_BOOL,             "false",           false,     "exclude from game counter",  "enter don't count as game off/on", false,   {}},
{"nomultiscrape", MD_BOOL,             "false",           false,     "exclude from multi-scraper", "enter no multi-scrape off/on",     false,   {}},
{"hidemetadata",  MD_BOOL,             "false",           false,     "hide metadata fields",       "enter hide metadata off/on",       false,   {}},
{"playcount",     MD_INT,              "0",               false,     "times played",               "enter number of times played",     false,   {}},
{"altemulator",   MD_ALT_EMULATOR,     "",                false,     "alternative emulator",       "select alternative emulator",      false,   {}},
{"lastplayed",    MD_TIME,             "0",               true,      "last played",                "enter last played date",           false,   {}},
{"controllers",    MD_STRING,           "",               false,     "controllers",                "select controllers",               false,   {"players"}}
};

MetaDataDecl folderDecls[] = {
{"name",          MD_STRING,           "",                false,     "name",                          "enter name",                       true,   {}},
{"desc",          MD_MULTILINE_STRING, "",                false,     "description",                   "enter description",                true,   {}},
{"rating",        MD_RATING,           "0",               false,     "rating",                        "enter rating",                     true,   {}},
{"releasedate",   MD_DATE,             "19700101T010000", false,     "release date",                  "enter release date",               true,   {}},
{"developer",     MD_STRING,           "unknown",         false,     "developer",                     "enter developer",                  true,   {}},
{"publisher",     MD_STRING,           "unknown",         false,     "publisher",                     "enter publisher",                  true,   {}},
{"genre",         MD_STRING,           "unknown",         false,     "genre",                         "enter genre",                      true,   {}},
{"players",       MD_STRING,           "unknown",         false,     "players",                       "enter number of players",          true,   {}},
{"favorite",      MD_BOOL,             "false",           false,     "favorite",                      "enter favorite off/on",            false,   {}},
{"completed",     MD_BOOL,             "false",           false,     "completed",                     "enter completed off/on",           false,   {}},
{"kidgame",       MD_BOOL,             "false",           false,     "kidgame (only affects badges)", "enter kidgame off/on",             false,   {}},
{"hidden",        MD_BOOL,             "false",           false,     "hidden",                        "enter hidden off/on",              false,   {}},
{"broken",        MD_BOOL,             "false",           false,     "broken/not working",            "enter broken off/on",              false,   {}},
{"nomultiscrape", MD_BOOL,             "false",           false,     "exclude from multi-scraper",    "enter no multi-scrape off/on",     false,   {}},
{"hidemetadata",  MD_BOOL,             "false",           false,     "hide metadata fields",          "enter hide metadata off/on",       false,   {}},
{"lastplayed",    MD_TIME,             "0",               true,      "last played",                   "enter last played date",           false,   {}}
};
// clang-format on

const std::vector<MetaDataDecl> gameMDD(gameDecls,
                                        gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

const std::vector<MetaDataDecl> folderMDD(folderDecls,
                                          folderDecls +
                                              sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
    switch (type) {
        case GAME_METADATA:
            return gameMDD;
        case FOLDER_METADATA:
            return folderMDD;
    }

    LOG(LogError) << "Invalid MDD type";
    return gameMDD;
}

MetaDataList::MetaDataList(MetaDataListType type)
    : mType(type)
    , mWasChanged(false)
{
    const std::vector<MetaDataDecl>& mdd = getMDD();
    for (auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
        set(iter->key, iter->defaultValue, {});
}

MetaDataList MetaDataList::createFromXML(MetaDataListType type,
                                         pugi::xml_node& node,
                                         const std::string& relativeTo)
{
    MetaDataList mdl(type);

    const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

    for (auto iter = mdd.cbegin(); iter != mdd.cend(); iter++) {

        // Find all nodes of given mdd type.
        const pugi::xml_object_range<pugi::xml_node_iterator>& children = node.children();
        bool found = false;
        std::string value = iter->defaultValue;
        std::map<std::string, std::map<std::string, std::string>> attr = {};
        for (auto iter2 = children.begin(); iter2 != children.end(); iter2++) {
            if (iter2->name() != iter->key)
                continue;

            // Parse value.
            std::string v = iter2->text().get();
            if (iter->type == MD_PATH)
                value = Utils::FileSystem::resolveRelativePath(v, relativeTo, true);
            if (iter->attr.empty())
                value = v;

            // Parse attributes.
            for (const auto& attrName : iter->attr) {
                // Initialize attribute to empty map if it doesn't exist yet.
                if (attr.find(attrName) == attr.end()) {
                    std::map<std::string, std::string> m = {};
                    attr[attrName] = m;
                }

                std::string attrValue = iter2->attribute(attrName.c_str()).as_string("");
                if (!attrValue.empty()) {
                    attr[attrName].insert({attrValue, v});
                }
            }
        }

        // Set to default value if no node was found.
        mdl.set(iter->key, value, attr);
    }
    return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent,
                               bool ignoreDefaults,
                               const std::string& relativeTo) const
{
    const std::vector<MetaDataDecl>& mdd = getMDD();

    for (auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++) {
        auto mapIter = mMap.find(mddIter->key);
        if (mapIter != mMap.cend()) {

            // We have this value!
            // If it's just the default (and we ignore defaults), don't write it.
            if (ignoreDefaults && mapIter->second.value == mddIter->defaultValue)
                continue;

            // Try and make paths relative if we can.
            std::string value = mapIter->second.value;
            if (mddIter->type == MD_PATH)
                value = Utils::FileSystem::createRelativePath(value, relativeTo, true);

            // Create simple node without attributes when we have a non-empty 'value'.
            if (!value.empty()) {
                auto node = parent.append_child(mapIter->first.c_str());
                node.text().set(value.c_str());
            }

            // Create nodes with attributes for every attribute value.
            for (auto& a : mapIter->second.attr) {
                for (auto& b : a.second) {
                    auto node = parent.append_child(mapIter->first.c_str());
                    auto nodeAttr = node.append_attribute(a.first.c_str());
                    nodeAttr.set_value(b.first.c_str());
                    node.text().set(b.second.c_str());
                }
            }
        }
    }
}

void MetaDataList::set(const std::string& key,
                       const std::string& value,
                       const std::map<std::string, std::map<std::string, std::string>>& attr)
{
    MetaDataValue mdv = {value, attr};
    mMap[key] = mdv;
    mWasChanged = true;
}

const MetaDataValue& MetaDataList::getWithAttributes(const std::string& key) const
{
    // Check that the key actually exists, otherwise return an empty string.
    if (mMap.count(key) > 0)
        return mMap.at(key);
    else
        return mNoResult;
}

const std::string& MetaDataList::get(const std::string& key) const
{
    // Check that the key actually exists, otherwise return an empty string.
    if (mMap.count(key) > 0)
        return mMap.at(key).value;
    else
        return mNoResult.value;
}

int MetaDataList::getInt(const std::string& key) const
{
    // Return integer value.
    return atoi(getWithAttributes(key).value.c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
    // Return float value.
    return static_cast<float>(atof(getWithAttributes(key).value.c_str()));
}

bool MetaDataList::wasChanged() const
{
    // Return whether the metadata was changed.
    return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
    // Reset the change flag.
    mWasChanged = false;
}

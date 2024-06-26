//  SPDX-License-Identifier: MIT
//
//  ES-DE
//  MetaData.cpp
//
//  Static data for default metadata values as well as functions
//  to read and write metadata from the gamelist files.
//

#include "MetaData.h"

#include "Log.h"
#include "utils/FileSystemUtil.h"

#include <pugixml.hpp>

namespace
{
    // clang-format off
    // The statistic entries must be placed at the bottom or otherwise there will be problems with
    // saving the values in GuiMetaDataEd.
    MetaDataDecl gameDecls[] {
    // Key                 Type                 Default value      Statistic  Name in GuiMetaDataEd          Prompt in GuiMetaDataEd             Scrape
    {"name",               MD_STRING,           "",                false,     "name",                        "enter name",                       true},
    {"sortname",           MD_STRING,           "",                false,     "sortname",                    "enter sortname",                   false},
    {"collectionsortname", MD_STRING,           "",                false,     "custom collections sortname", "enter collections sortname",       false},
    {"desc",               MD_MULTILINE_STRING, "",                false,     "description",                 "enter description",                true},
    {"rating",             MD_RATING,           "0",               false,     "rating",                      "enter rating",                     true},
    {"releasedate",        MD_DATE,             "19700101T000000", false,     "release date",                "enter release date",               true},
    {"developer",          MD_STRING,           "unknown",         false,     "developer",                   "enter developer",                  true},
    {"publisher",          MD_STRING,           "unknown",         false,     "publisher",                   "enter publisher",                  true},
    {"genre",              MD_STRING,           "unknown",         false,     "genre",                       "enter genre",                      true},
    {"players",            MD_STRING,           "unknown",         false,     "players",                     "enter number of players",          true},
    {"favorite",           MD_BOOL,             "false",           false,     "favorite",                    "enter favorite off/on",            false},
    {"completed",          MD_BOOL,             "false",           false,     "completed",                   "enter completed off/on",           false},
    {"kidgame",            MD_BOOL,             "false",           false,     "kidgame",                     "enter kidgame off/on",             false},
    {"hidden",             MD_BOOL,             "false",           false,     "hidden",                      "enter hidden off/on",              false},
    {"broken",             MD_BOOL,             "false",           false,     "broken/not working",          "enter broken off/on",              false},
    {"nogamecount",        MD_BOOL,             "false",           false,     "exclude from game counter",   "enter don't count as game off/on", false},
    {"nomultiscrape",      MD_BOOL,             "false",           false,     "exclude from multi-scraper",  "enter no multi-scrape off/on",     false},
    {"hidemetadata",       MD_BOOL,             "false",           false,     "hide metadata fields",        "enter hide metadata off/on",       false},
    {"playcount",          MD_INT,              "0",               false,     "times played",                "enter number of times played",     false},
    {"controller",         MD_CONTROLLER,       "",                false,     "controller",                  "select controller",                true},
    {"altemulator",        MD_ALT_EMULATOR,     "",                false,     "alternative emulator",        "select alternative emulator",      false},
    {"lastplayed",         MD_TIME,             "0",               true,      "last played",                 "enter last played date",           false}
    };

    MetaDataDecl folderDecls[] {
    // Key            Type                 Default value      Statistic  Name in GuiMetaDataEd            Prompt in GuiMetaDataEd             Scrape
    {"name",          MD_STRING,           "",                false,     "name",                          "enter name",                       true},
    {"desc",          MD_MULTILINE_STRING, "",                false,     "description",                   "enter description",                true},
    {"rating",        MD_RATING,           "0",               false,     "rating",                        "enter rating",                     true},
    {"releasedate",   MD_DATE,             "19700101T000000", false,     "release date",                  "enter release date",               true},
    {"developer",     MD_STRING,           "unknown",         false,     "developer",                     "enter developer",                  true},
    {"publisher",     MD_STRING,           "unknown",         false,     "publisher",                     "enter publisher",                  true},
    {"genre",         MD_STRING,           "unknown",         false,     "genre",                         "enter genre",                      true},
    {"players",       MD_STRING,           "unknown",         false,     "players",                       "enter number of players",          true},
    {"favorite",      MD_BOOL,             "false",           false,     "favorite",                      "enter favorite off/on",            false},
    {"completed",     MD_BOOL,             "false",           false,     "completed",                     "enter completed off/on",           false},
    {"kidgame",       MD_BOOL,             "false",           false,     "kidgame (only affects badges)", "enter kidgame off/on",             false},
    {"hidden",        MD_BOOL,             "false",           false,     "hidden",                        "enter hidden off/on",              false},
    {"broken",        MD_BOOL,             "false",           false,     "broken/not working",            "enter broken off/on",              false},
    {"nomultiscrape", MD_BOOL,             "false",           false,     "exclude from multi-scraper",    "enter no multi-scrape off/on",     false},
    {"hidemetadata",  MD_BOOL,             "false",           false,     "hide metadata fields",          "enter hide metadata off/on",       false},
    {"controller",    MD_CONTROLLER,       "",                false,     "controller",                    "select controller",                true},
    {"folderlink",    MD_FOLDER_LINK,      "",                false,     "folder link",                   "select folder link",               false},
    {"lastplayed",    MD_TIME,             "0",               true,      "last played",                   "enter last played date",           false}
    };
    // clang-format on

    const std::vector<MetaDataDecl> gameMDD {gameDecls,
                                             gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0])};

    const std::vector<MetaDataDecl> folderMDD {
        folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0])};

} // namespace

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
    for (auto it = mdd.cbegin(); it != mdd.cend(); ++it)
        set(it->key, it->defaultValue);
}

MetaDataList MetaDataList::createFromXML(MetaDataListType type,
                                         pugi::xml_node& node,
                                         const std::string& relativeTo)
{
    MetaDataList mdl(type);

    const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

    for (auto it = mdd.cbegin(); it != mdd.cend(); ++it) {
        pugi::xml_node md = node.child(it->key.c_str());
        if (md && !md.text().empty()) {
            // If it's a path, resolve relative paths.
            std::string value = md.text().get();
            if (it->type == MD_PATH)
                value = Utils::FileSystem::resolveRelativePath(value, relativeTo, true);
            mdl.set(it->key, value);
        }
        else {
            mdl.set(it->key, it->defaultValue);
        }
    }
    return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent,
                               bool ignoreDefaults,
                               const std::string& relativeTo) const
{
    const std::vector<MetaDataDecl>& mdd = getMDD();

    for (auto it = mdd.cbegin(); it != mdd.cend(); ++it) {
        auto mapIter = mMap.find(it->key);
        if (mapIter != mMap.cend()) {
            // We have this value!
            // If it's just the default (and we ignore defaults), don't write it.
            if (ignoreDefaults && mapIter->second == it->defaultValue)
                continue;

            // Try and make paths relative if we can.
            std::string value = mapIter->second;
            if (it->type == MD_PATH)
                value = Utils::FileSystem::createRelativePath(value, relativeTo, true);

            parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
        }
    }
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
    mMap[key] = value;
    mWasChanged = true;
}

const std::string& MetaDataList::get(const std::string& key) const
{
    // Check that the key actually exists, otherwise return an empty string.
    if (mMap.count(key) > 0)
        return mMap.at(key);
    else
        return mNoResult;
}

int MetaDataList::getInt(const std::string& key) const
{
    // Return integer value.
    return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
    // Return float value.
    return static_cast<float>(atof(get(key).c_str()));
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

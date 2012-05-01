#include "Xml.h"

UserdataStubs(Xml, mxml_node_t*);

static int luaXml_new(lua_State *L)
{
    L_CONST char *verString = (lua_gettop(L) >= 1) ? luaL_checkstring(L,1) : "1.0";
    *pushXml(L) = mxmlNewXML(verString);
    return 1;
}

static int luaXml_newElement(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:newElement(name) takes 1 argument and must be called with a colon.");
    }
    *pushXml(L) = mxmlNewElement(*toXml(L,1), luaL_checkstring(L,2));
    return 1;
}

static int luaXml_setText(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:setText(text) takes 1 argument and must be called with a colon.");
    }
    L_CONST char *text = luaL_checkstring(L,2);
    mxmlNewText(*toXml(L,1), (strstr(text, " ")) ? 1 : 0, text);
    return 0;
}

static int luaXml_setInt(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:setInt(integer) takes 1 argument and must be called with a colon.");
    }
    mxmlNewInteger(*toXml(L,1),luaL_checkint(L,2));
    return 0;
}

static int luaXml_setNum(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:setNum(number) takes 1 argument and must be called with a colon.");
    }
    double n = luaL_checknumber(L, 2);
    mxmlSetReal(*toXml(L,1),n);
    return 0;
}

static int luaXml_load(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml.load(filename) takes 1 argument.");
    }
    L_CONST char *filename = luaL_checkstring(L,1);
    FILE *fp = fopen(filename, "r");
    if(!fp)
    {
        return luaL_error(L, "Cannot load the xml file '%s'.", filename);
    }
    *pushXml(L) = mxmlLoadFile(null, fp, MXML_TEXT_CALLBACK);
    fclose(fp);

    return 1;
}

static int luaXml_save(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:save(filename) takes 1 argument and must be called with a colon.");
    }
    L_CONST char *filename = luaL_checkstring(L,2);
    FILE *fp = fopen(filename, "w");
    if(!fp)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    mxmlSaveFile(*toXml(L,1), fp, MXML_NO_CALLBACK);
    fclose(fp);
    lua_pushboolean(L, 1);
    return 1;
}

static int luaXml_setWrapMargin(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml.setWrapMargin(margin) takes 1 argument.");
    }
    mxmlSetWrapMargin(luaL_checkint(L,1));
    return 0;
}

static int luaXml_findNode(lua_State *L)
{
    Int16 argc = lua_gettop(L);
    if(argc < 2 || argc > 5)
    {
        return luaL_error(L, "Xml:findNode(top, [name], [attr], [value]) takes a maximum of 3 arguments and must be called with a colon.");
    }
    L_CONST char *name = lua_isstring(L,3) ? luaL_checkstring(L,3) : null;
    L_CONST char *attr = argc >= 4 && lua_isstring(L,4) ? luaL_checkstring(L,4) : null;
    L_CONST char *value = argc >= 5 && lua_isstring(L,5) ? luaL_checkstring(L,5) : null;

    *pushXml(L) = mxmlFindElement(*toXml(L,1), *toXml(L,2), name, attr, value, MXML_DESCEND);

    return 1;
}

static int luaXml_findPath(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:findPath(path) takes 1 argument and must be called with a colon.");
    }
    *pushXml(L) = mxmlFindPath(*toXml(L,1), luaL_checkstring(L,2));
    return 1;
}

static int luaXml_prev(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:prev(top) takes 1 argument and must be called with a colon.");
    }
    *pushXml(L) = mxmlWalkPrev(*toXml(L,1), *toXml(L,2), MXML_DESCEND);

    return 1;
}

static int luaXml_next(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:next(top) takes 1 argument and must be called with a colon.");
    }
    *pushXml(L) = mxmlWalkNext(*toXml(L,1), *toXml(L,2), MXML_DESCEND);

    return 1;
}

static int luaXml_getText(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getText() takes no arguments and must be called with a colon.");
    }
    int ws = 0;
    lua_pushstring(L, mxmlGetText(*toXml(L,1), &ws));
    lua_pushboolean(L, ws);
    return 2;
}

static int luaXml_getInt(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getInt() takes no arguments and must be called with a colon.");
    }
    lua_pushnumber(L, mxmlGetInteger(*toXml(L,1)));
    return 1;
}

static int luaXml_getNum(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getNum() takes no arguents and must be called with a colon.");
    }
    lua_pushnumber(L, mxmlGetReal(*toXml(L,1)));
    return 1;
}

static int luaXml_getElement(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getElement() takes no arguments and must be called with a colon.");
    }
    lua_pushstring(L, mxmlGetElement(*toXml(L,1)));
    return 1;
}

static int luaXml_getFirstChild(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getFirstChild() takes no arguments and must be called with a colon.");
    }
    *pushXml(L) = mxmlGetFirstChild(*toXml(L,1));
    return 1;
}

static int luaXml_getLastChild(lua_State *L)
{
    if(lua_gettop(L) != 1)
    {
        return luaL_error(L, "Xml:getLastChild() takes no arguments and must be called with a colon.");
    }
    *pushXml(L) = mxmlGetLastChild(*toXml(L,1));
    return 1;
}

static int luaXml_getAttr(lua_State *L)
{
    if(lua_gettop(L) != 2)
    {
        return luaL_error(L, "Xml:getAttr(name) takes 1 argument and must be called with a colon.");
    }
    lua_pushstring(L, mxmlElementGetAttr(*toXml(L,1), luaL_checkstring(L,2)));
    return 1;
}

static int luaXml_setAttr(lua_State *L)
{
    if(lua_gettop(L) != 3)
    {
        return luaL_error(L, "Xml:setAttr(name, attr) takes 2 arguments and must be called with a colon.");
    }
    mxmlElementSetAttr(*toXml(L,1), luaL_checkstring(L, 2), luaL_checkstring(L, 3));
    return 0;
}

static int luaXml__gc(lua_State *L)
{
    mxmlDelete(*toXml(L,1));
    return 0;
}

static L_CONST luaL_reg luaXml_methods[] = {
    { "new", luaXml_new },
    { "newElement", luaXml_newElement },
    { "setText", luaXml_setText },
    { "setInt", luaXml_setInt },
    { "setNum", luaXml_setNum },
    { "load", luaXml_load },
    { "save", luaXml_save },
    { "setWrapMargin", luaXml_setWrapMargin },
    { "findNode", luaXml_findNode },
    { "findPath", luaXml_findPath },
    { "prev", luaXml_prev },
    { "next", luaXml_next },
    { "getText", luaXml_getText },
    { "getInt", luaXml_getInt },
    { "getNum", luaXml_getNum },
    { "getElement", luaXml_getElement },
    { "getFirstChild", luaXml_getFirstChild },
    { "getLastChild", luaXml_getLastChild },
    { "getAttr", luaXml_getAttr },
    { "setAttr", luaXml_setAttr },
    { 0, 0 }
};

static L_CONST luaL_reg luaXml_meta[] = {
    { "__gc", luaXml__gc },
    { 0, 0 }
};

UserdataRegister(Xml, luaXml_methods, luaXml_methods);

void luaXml_Init(lua_State *L)
{
    Xml_register(L);
}

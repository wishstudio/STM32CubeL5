/* DO NOT EDIT THIS FILE */
/* This file is autogenerated by the text-database code generator */

#ifndef TYPEDTEXTDATABASE_HPP
#define TYPEDTEXTDATABASE_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/TypedText.hpp>

namespace TypedTextDatabase
{
class TypedTextData;
const touchgfx::TypedText::TypedTextData* getInstance(touchgfx::LanguageId id);
const touchgfx::TypedText::TypedTextData* getInstance();
const touchgfx::Font** getFonts();
const touchgfx::Font* setFont(touchgfx::FontId fontId, const touchgfx::Font*);
void resetFont(touchgfx::FontId fontId);
uint16_t getInstanceSize();
}

#endif

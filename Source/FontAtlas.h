#pragma once
#include "Primitive/PrimitiveVertices.h"
#include "Core/Container/Map.h"
#include "Core/AbstractClass/Singleton.h"

class FFontAtlas : public TSingleton<FFontAtlas>
{
public:
	FFontAtlas();

	const GlyphInfo& GetGlyph(wchar_t c) const
	{
		auto it = glyphs.Find(c);
		if (it == nullptr)
		{
			return defaultGlyph;
		}
		return *it;
	}

	void AddGlyph(wchar_t c, const GlyphInfo& glyph)
	{
		glyphs[c] = glyph;
	}

private:
	TMap<wchar_t, GlyphInfo> glyphs;
	GlyphInfo defaultGlyph = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
};
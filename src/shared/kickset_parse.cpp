#include "KickSet.h"

static void ParseError(const char* str)
{
	Debug::PrintLine(CSTR("[KickSetParser] ", str));

}

Hashmap<String, EKickRotTransfer> string2transfer =
{
	Pair<const String, EKickRotTransfer>("0->90",	EKickRotTransfer::START2RIGHT),
	Pair<const String, EKickRotTransfer>("0->180",	EKickRotTransfer::START2FLIP),
	Pair<const String, EKickRotTransfer>("0->270",	EKickRotTransfer::START2LEFT),

	Pair<const String, EKickRotTransfer>("90->0",	EKickRotTransfer::RIGHT2START),
	Pair<const String, EKickRotTransfer>("90->180",	EKickRotTransfer::RIGHT2FLIP),
	Pair<const String, EKickRotTransfer>("90->270", EKickRotTransfer::RIGHT2LEFT),

	Pair<const String, EKickRotTransfer>("180->0",	EKickRotTransfer::FLIP2START),
	Pair<const String, EKickRotTransfer>("180->90", EKickRotTransfer::FLIP2RIGHT),
	Pair<const String, EKickRotTransfer>("180->270",EKickRotTransfer::FLIP2LEFT),

	Pair<const String, EKickRotTransfer>("270->0",	EKickRotTransfer::LEFT2START),
	Pair<const String, EKickRotTransfer>("270->90", EKickRotTransfer::LEFT2RIGHT),
	Pair<const String, EKickRotTransfer>("270->180",EKickRotTransfer::LEFT2FLIP),
};

bool KickSet::Parse(const char* const string)
{
	List<PartID> parts;
	List<EKickRotTransfer> rots;

	RotTransferData* currentData = nullptr;
	bool inSquareSet = false;
	const char* c = string;
	bool inOffsetScope = false;

	while (true)
	{
		while (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n' || *c == ';')
			++c;

		if (*c == '\0')
			break;

		if (*c == '[')
		{
			if (inSquareSet)
			{
				ParseError("Nested square brackets are not supported!");
				return false;
			}

			inSquareSet = true;

			if (currentData)
				rots.Clear();
			else
				parts.Clear();
				
		}
		else if (*c == ']')
		{
			if (!inSquareSet)
			{
				ParseError("\']\' when outside of scope!");
				return false;
			}

			inSquareSet = false;
		}
		else if (*c == '{')
		{
			if (!currentData)
			{
				currentData = &rotSets.Emplace();
				for (PartID part : parts)
					part2rotSetIdx.Set(part, rotSets.GetSize() - 1);
			}
			else if (!inOffsetScope)
			{
				inOffsetScope = true;
			}
			else
			{
				ParseError("\'{\' scopes deeper than offset lists are not supported..");
				return false;
			}
		}
		else if (*c == '}')
		{
			if (inOffsetScope)
			{
				rots.Clear();
				inOffsetScope = false;
			}
			else if (currentData)
				currentData = nullptr;
			else
				ParseError("\'}\' is outside of scope");
		}
		else
		{
			const char* wordStart = c;

			while (true)
			{
				if (*c == '\0' ||
					*c == ' ' ||
					*c == '\t' ||
					*c == '\r' ||
					*c == '\n' ||
					*c == ';' ||
					*c == ']')
					break;

				++c;
			}

			if (currentData)
			{
				if (inSquareSet)
				{
					EKickRotTransfer* transfer = string2transfer.TryGet(String(wordStart, c - wordStart));
					if (transfer)
						rots.EmplaceBack(*transfer);
					else
						ParseError(CSTR('\"', String(wordStart, c - wordStart), "\" is not a valid rotation!"));

					continue;
				}

				if (inOffsetScope)
				{
					Vector2 offset;
					int element = 0;
					const char* elemStart = wordStart;
					const char* elemEnd = wordStart;

					while (true)
					{
						const bool end = *elemEnd == '\0' || *elemEnd == '\r' || *elemEnd == '\n' || *elemEnd == ';';

						if (*elemEnd == ',' || end)
						{
							if (element < 2)
							{
								offset[element++] = String(elemStart, elemEnd - elemStart).ToFloat();
								elemStart = elemEnd + 1;
							}
						}

						if (end)
							break;

						++elemEnd;
					}

					c = elemEnd;

					if (element >= 2)
						for (EKickRotTransfer rot : rots)
							currentData->TestList(rot).EmplaceBack(offset);
				}
				else
					ParseError(CSTR("\'", String(wordStart, c - wordStart), "\' is not in offset scope!!"));

			}
			else if (inSquareSet)
			{
				parts.EmplaceBack((int)*wordStart);
			}
			else
			{
				ParseError(CSTR("\'", String(wordStart, c - wordStart), "\' is out of scope!!"));
			}

			continue; //dont need the increment here
		}

		++c;
	}

	return true;
}

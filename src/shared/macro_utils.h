#pragma once

#define IF_NOT_FIRST_EXEC_DO(BODY) { static bool ___first___ = true; if (___first___) ___first___ = false; else BODY; }
#define IF_FIRST_EXEC_DO(BODY) { static bool ___first___ = true; if (___first___) BODY; ___first___ = false; }

#define DEFINE_BITMASK_FUNCS(ENUMCLASS, NUMERICALTYPE)				\
																	\
inline ENUMCLASS operator|(const ENUMCLASS& a, const ENUMCLASS& b)	\
{																	\
	return ENUMCLASS((NUMERICALTYPE)a | (NUMERICALTYPE)b);			\
}																	\
																	\
inline ENUMCLASS operator&(const ENUMCLASS& a, const ENUMCLASS& b)	\
{																	\
	return ENUMCLASS((NUMERICALTYPE)a & (NUMERICALTYPE)b);			\
}																	\
																	\
inline ENUMCLASS operator|=(ENUMCLASS& a, const ENUMCLASS& b)		\
{																	\
	return a = a | b;												\
}																	\
																	\
inline ENUMCLASS operator&=(ENUMCLASS& a, const ENUMCLASS& b)		\
{																	\
	return ENUMCLASS((NUMERICALTYPE)a & (NUMERICALTYPE)b);			\
}																	\
																	\
inline ENUMCLASS operator~(const ENUMCLASS& rc)						\
{																	\
	return ENUMCLASS(~(NUMERICALTYPE)rc);							\
}																	

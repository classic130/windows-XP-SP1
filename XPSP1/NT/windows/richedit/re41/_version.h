#define RICHEDIT_VER "5.41.15.1503"
#define RICHEDIT_VERMAJ 41
#define RICHEDIT_VERMIN 15
#define RICHEDIT_VERBUILD 1503
#ifdef DEBUG
#define RICHEDIT_BUILD RICHEDIT_VER ## " (Debug)"
#else
#define RICHEDIT_BUILD RICHEDIT_VER
#endif
#define RICHEDIT_HEADER "Msftedit " ## RICHEDIT_BUILD
#define RTF_GENINFO "{\\*\\generator " ## RICHEDIT_HEADER ## ";}"

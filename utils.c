// Utils by OrionSoft [2024] https://orionsoft.fr/

#include "utils.h"
#include "main.h"

/****************************************/

char	*StrCmpRef(char *refstr, char *str)
{
	if (strncmp(refstr, str, strlen(refstr)) == 0)	// Si on trouve la chaine de réference
		return (&str[strlen(refstr)]);				// Renvoi un pointeur après cette référence
	return (NULL);
}


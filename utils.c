// Utils by OrionSoft [2024] https://orionsoft.fr/

#include "utils.h"
#include "main.h"

/****************************************/

char	*StrCmpRef(char *refstr, char *str)
{
	if (strncmp(refstr, str, strlen(refstr)) == 0)	// Si on trouve la chaine de r�ference
		return (&str[strlen(refstr)]);				// Renvoi un pointeur apr�s cette r�f�rence
	return (NULL);
}


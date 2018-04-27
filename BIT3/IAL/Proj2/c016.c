/* Martina Grzybowska - xgrzyb00 */

/* c016.c: **********************************************************}
{* Téma:  Tabulka s Rozptýlenými Položkami
**                      První implementace: Petr Přikryl, prosinec 1994
**                      Do jazyka C prepsal a upravil: Vaclav Topinka, 2005
**                      Úpravy: Karel Masařík, říjen 2014
**                              Radek Hranický, říjen 2014
**                              Radek Hranický, listopad 2015
**                              Radek Hranický, říjen 2016
**
** Vytvořete abstraktní datový typ
** TRP (Tabulka s Rozptýlenými Položkami = Hash table)
** s explicitně řetězenými synonymy. Tabulka je implementována polem
** lineárních seznamů synonym.
**
** Implementujte následující procedury a funkce.
**
**  HTInit ....... inicializuje tabulku před prvním použitím
**  HTInsert ..... vložení prvku
**  HTSearch ..... zjištění přítomnosti prvku v tabulce
**  HTDelete ..... zrušení prvku
**  HTRead ....... přečtení hodnoty prvku
**  HTClearAll ... zrušení obsahu celé tabulky (inicializace tabulky
**                 poté, co již byla použita)
**
** Definici typů naleznete v souboru c016.h.
**
** Tabulka je reprezentována datovou strukturou typu tHTable,
** která se skládá z ukazatelů na položky, jež obsahují složky
** klíče 'key', obsahu 'data' (pro jednoduchost typu float), a
** ukazatele na další synonymum 'ptrnext'. Při implementaci funkcí
** uvažujte maximální rozměr pole HTSIZE.
**
** U všech procedur využívejte rozptylovou funkci hashCode.  Povšimněte si
** způsobu předávání parametrů a zamyslete se nad tím, zda je možné parametry
** předávat jiným způsobem (hodnotou/odkazem) a v případě, že jsou obě
** možnosti funkčně přípustné, jaké jsou výhody či nevýhody toho či onoho
** způsobu.
**
** V příkladech jsou použity položky, kde klíčem je řetězec, ke kterému
** je přidán obsah - reálné číslo.
*/

#include "c016.h"

int HTSIZE = MAX_HTSIZE;
int solved;

/*          -------
** Rozptylovací funkce - jejím úkolem je zpracovat zadaný klíč a přidělit
** mu index v rozmezí 0..HTSize-1.  V ideálním případě by mělo dojít
** k rovnoměrnému rozptýlení těchto klíčů po celé tabulce.  V rámci
** pokusů se můžete zamyslet nad kvalitou této funkce.  (Funkce nebyla
** volena s ohledem na maximální kvalitu výsledku). }
*/

int hashCode ( tKey key ) {
	int retval = 1;
	int keylen = strlen(key);
	for ( int i=0; i<keylen; i++ )
		retval += key[i];
	return ( retval % HTSIZE );
}

/*
** Inicializace tabulky s explicitně zřetězenými synonymy.  Tato procedura
** se volá pouze před prvním použitím tabulky.
*/

void htInit ( tHTable* ptrht ) {

	if ((*ptrht) == NULL)
		return;

	for (int a = 0; a < HTSIZE; a++)
		((*ptrht)[a] = NULL);

	return;
 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

/* TRP s explicitně zřetězenými synonymy.
** Vyhledání prvku v TRP ptrht podle zadaného klíče key.  Pokud je
** daný prvek nalezen, vrací se ukazatel na daný prvek. Pokud prvek nalezen není, 
** vrací se hodnota NULL.
**
*/

tHTItem* htSearch ( tHTable* ptrht, tKey key ) {

	if ((*ptrht) == NULL)
		return NULL;

	tHTItem* tmpAid = (*ptrht)[hashCode(key)]; //tmpAid ukaze na dany index pola

	if (tmpAid != NULL)
	{
		while (tmpAid != NULL) //prehladavanie synonym
			if (strcmp(tmpAid->key, key) == 0)
				return tmpAid;
			else
				tmpAid = tmpAid->ptrnext;
	}
	return NULL;

 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

/* 
** TRP s explicitně zřetězenými synonymy.
** Tato procedura vkládá do tabulky ptrht položku s klíčem key a s daty
** data.  Protože jde o vyhledávací tabulku, nemůže být prvek se stejným
** klíčem uložen v tabulce více než jedenkrát.  Pokud se vkládá prvek,
** jehož klíč se již v tabulce nachází, aktualizujte jeho datovou část.
**
** Využijte dříve vytvořenou funkci htSearch.  Při vkládání nového
** prvku do seznamu synonym použijte co nejefektivnější způsob,
** tedy proveďte.vložení prvku na začátek seznamu.
**/

void htInsert ( tHTable* ptrht, tKey key, tData data ) {

	if ((*ptrht) == NULL)
		return;

	tHTItem* tmpAid = htSearch(ptrht,key);

	if (tmpAid != NULL)
		tmpAid->data = data;
	else
	{
		tHTItem* newItem = malloc(sizeof(struct tHTItem));
		if (newItem != NULL)
		{
			int tmpHlp = hashCode(key);
			newItem->data = data;
			newItem->key = key;
			newItem->ptrnext = (*ptrht)[tmpHlp];
			(*ptrht)[tmpHlp] = newItem;
		}
	}
	return;

 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

/*
** TRP s explicitně zřetězenými synonymy.
** Tato funkce zjišťuje hodnotu datové části položky zadané klíčem.
** Pokud je položka nalezena, vrací funkce ukazatel na položku
** Pokud položka nalezena nebyla, vrací se funkční hodnota NULL
**
** Využijte dříve vytvořenou funkci HTSearch.
*/

tData* htRead ( tHTable* ptrht, tKey key ) {

	if ((*ptrht) == NULL)
		return NULL;

	tHTItem* tmpAid = htSearch(ptrht,key);

	if (tmpAid != NULL)
		return &tmpAid->data;
	else
		return NULL;

 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

/*
** TRP s explicitně zřetězenými synonymy.
** Tato procedura vyjme položku s klíčem key z tabulky
** ptrht.  Uvolněnou položku korektně zrušte.  Pokud položka s uvedeným
** klíčem neexistuje, dělejte, jako kdyby se nic nestalo (tj. nedělejte
** nic).
**
** V tomto případě NEVYUŽÍVEJTE dříve vytvořenou funkci HTSearch.
*/

void htDelete ( tHTable* ptrht, tKey key ) {

	if ((*ptrht) == NULL)
		return;

	tHTItem* tmpAid = (*ptrht)[hashCode(key)];
	tHTItem* tmpAidSwitch = NULL;

	if (tmpAid != NULL)
	{
		while (tmpAid != NULL)
		{ 
			if (strcmp(tmpAid->key, key) == 0)
			{
				if (tmpAidSwitch == NULL)
					(*ptrht)[hashCode(key)] = tmpAid->ptrnext;
				else
					tmpAidSwitch->ptrnext = tmpAid->ptrnext;
				
				free(tmpAid);
				return;
			}
			else
			{
				tmpAidSwitch = tmpAid;
				tmpAid = tmpAid->ptrnext;
			}
		}
	}
	return;
 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

/* TRP s explicitně zřetězenými synonymy.
** Tato procedura zruší všechny položky tabulky, korektně uvolní prostor,
** který tyto položky zabíraly, a uvede tabulku do počátečního stavu.
*/

void htClearAll ( tHTable* ptrht ) {

	if ((*ptrht) == NULL)
		return;

	for (int i = 0; i < HTSIZE; i++)
	{
		tHTItem* tmpAid = (*ptrht)[i];
		tHTItem* tmpAidSwitch;

			while(tmpAid != NULL)
			{
				tmpAidSwitch = tmpAid;
				tmpAid = tmpAid->ptrnext;
				free(tmpAidSwitch);
				tmpAidSwitch = NULL;
			}
		(*ptrht)[i] = NULL;
	}
 //solved = 0; /*v pripade reseni, smazte tento radek!*/
}

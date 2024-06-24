#include "anlex.h"

/*********************HASH************************/
entrada *tabla;				//declarar la tabla de simbolos
int tamTabla=TAMHASH;		//utilizado para cuando se debe hacer rehash
int elems=0;				//utilizado para cuando se debe hacer rehash

int h(const char* k, int m)
{
	unsigned h=0,g;
	int i;
	for (i=0;i<strlen(k);i++)
	{
		h=(h << 4) + k[i];
		if ( (g=h&0xf0000000) ){
			h=h^(g>>24);
			h=h^g;
		}
	}
	return h%m;
}

void initTabla()
{	
	int i=0;
	
	tabla=(entrada*)malloc(tamTabla*sizeof(entrada));
	for(i=0;i<tamTabla;i++)
	{
		tabla[i].compLex=-1;
	}
}

int esprimo(int n)
{
	if(n == 2 || n == 3)
		return 1;
	if(n == 1 || n % 2 == 0)
		return 0;
	for(int i = 3; i * i <= n; i += 2)
		if(n % i == 0)
			return 0;
	return 1;
}

int siguiente_primo(int n)
{
	if (n%2==0)
		n++;
	for (;!esprimo(n);n+=2);

	return n;
}

//en caso de que la tabla llegue al limite, duplicar el tamaño
void rehash()
{
	entrada *vieja;
	int i;
	vieja=tabla;
	tamTabla=siguiente_primo(2*tamTabla);
	initTabla();
	for (i=0;i<tamTabla/2;i++)
	{
		if(vieja[i].compLex!=-1)
			insertar(vieja[i]);
	}		
	free(vieja);
}

//insertar una entrada en la tabla
void insertar(entrada e)
{
	int pos;
	if (++elems>=tamTabla/2)
		rehash();
	pos=h(e.lexema,tamTabla);
	while (tabla[pos].compLex!=-1)
	{
		pos++;
		if (pos==tamTabla)
			pos=0;
	}
	tabla[pos]=e;

}
//busca una clave en la tabla, si no existe devuelve NULL, posicion en caso contrario
entrada* buscar(const char *clave)
{
	int pos;
	pos=h(clave,tamTabla);
	while(tabla[pos].compLex!=-1 && strcmp(tabla[pos].lexema,clave)!=0 )
	{
		pos++;
		if (pos==tamTabla)
			pos=0;
	}
	return &tabla[pos];
}

void insertTablaSimbolos(const char *s, int n)
{
	entrada e;
	strcpy(e.lexema,s);
	// sprintf(e.lexema,s);
	e.compLex=n;
	insertar(e);
}

char* getTokenFromCode(int code) {
	switch (code) {
		case L_CORCHETE:
			return "L_CORCHETE";
		case R_CORCHETE:
			return "R_CORCHETE";
		case L_LLAVE:
			return "L_LLAVE";
		case R_LLAVE:
			return "R_LLAVE";
		case COMA:
			return "COMA";
		case DOS_PUNTOS:
			return "DOS_PUNTOS";
		case LITERAL_CADENA:
			return "LITERAL_CADENA";
		case LITERAL_NUM:
			return "LITERAL_NUM";
		case PR_TRUE:
			return "PR_TRUE";
		case PR_FALSE:
			return "PR_FALSE";
		case PR_NULL:
			return "PR_NULL";
		case EOF2:
			return "EOF";
		default:
			return "";
	}
}

void initTablaSimbolos()
{
	insertTablaSimbolos("[",L_CORCHETE);
	insertTablaSimbolos("]",R_CORCHETE);
	insertTablaSimbolos("{",L_LLAVE);
	insertTablaSimbolos("}",R_LLAVE);
	insertTablaSimbolos(",",COMA);
	insertTablaSimbolos(":",DOS_PUNTOS);
	insertTablaSimbolos("true",PR_TRUE);
	insertTablaSimbolos("TRUE",PR_TRUE);
	insertTablaSimbolos("false",PR_FALSE);
	insertTablaSimbolos("FALSE",PR_FALSE);
	insertTablaSimbolos("null",PR_NULL);
	insertTablaSimbolos("NULL",PR_NULL);
	insertTablaSimbolos("eof",EOF2);
}
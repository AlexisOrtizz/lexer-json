/*
 *	Analizador Sintáctico Descendente	
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Práctica de Programación Nro. 2
 *	
 *	Descripcion:
 *	 Implementar un analizador sintáctico descendente recursivo o LL(1) para 
 *	 el lenguaje Json simplificado.
 *	
 */

/*********** Inclusión de cabecera **************/
#include "anlex.h"


/************* Variables globales **************/

int consumir;			/* 1 indica al analizador lexico que debe devolver
						el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];		// string utilizado para cargar mensajes de error
token t;				// token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico

FILE *archivo;			// Fuente pascal
char buff[2*TAMBUFF];	// Buffer para lectura de archivo fuente
char lexema[TAMLEX];	// Utilizado por el analizador lexico
int delantero=-1;		// Utilizado por el analizador lexico
int fin=0;				// Utilizado por el analizador lexico
int sintaxError=0;		// Utilizado por el analizador sintactico
int numLinea=1;			// Numero de Linea

/**************** Funciones **********************/


// Rutinas del analizador lexico
void error(const char* mensaje);
void getToken();
void match(int expToken);

// Rutinas del analizador sintactico
void errorSyntax(const char* mensaje);
void json(int);
void element(int, int);
void element_list(int);
void element_listB(int);
void array(int);
void arrayB(int);
void object(int, int);
void objectB(int, int);
void attributes_list(int);
void attributes_listB(int);
void attribute(int);
char* attribute_name(int);
void attribute_value(int, char*);
void sincronizar(int expToken);

void printIndented(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void printToken(token t) {
    if (t.compLex == LITERAL_CADENA || t.compLex == LITERAL_NUM) {
        printf("%s", t.pe->lexema);
    } else if (t.compLex == PR_TRUE) {
        printf("true");
    } else if (t.compLex == PR_FALSE) {
        printf("false");
    } else if (t.compLex == PR_NULL) {
        printf("null");
    } else {
        printf("%s", getTokenFromCode(t.compLex));
    }
}


/* BEGIN: main */
int main(int argc,char* args[])
{
	int beforeLine = 0;
	initTabla();
	initTablaSimbolos();

	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}

		getToken();
		json(beforeLine);

		if (sintaxError == 0)
        {
			printf("El fuente es sintácticamente correcto.\n");
        }
        else
        {
            printf("El fuente es sintácticamente incorrecto.\n");
        }

		fclose(archivo);
	} else {
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}
/* END: main */

// BEGIN: Rutinas del analizador lexico

void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n", numLinea, mensaje);	
}

void getToken()
{
	int i=0;
	char c=0;
	int acepto=0;
	int estado=0;
	char msg[41];
	entrada e;

	while((c=fgetc(archivo))!=EOF)
	{
		
		if ( c==' ' || c=='\t' || c=='\r' )
			continue;	//eliminar espacios en blanco
		else if(c=='\n')
		{
			//incrementar el numero de linea
			numLinea++;
			continue;
		}
		// Reconocer símbolos JSON
		else if (c == '{') {
			t.compLex=L_LLAVE;
			t.pe=buscar("{");
			break;
		} 
		else if (c == '}') {
				t.compLex = R_LLAVE;
				t.pe=buscar("}");
				break;
		} 
		else if (c == '[') {
				t.compLex = L_CORCHETE;
				t.pe=buscar("[");
				break;
		} 
		else if (c == ']') {
				t.compLex = R_CORCHETE;
				t.pe=buscar("]");
				break;
		} 
		else if (c == ',') {
				t.compLex = COMA;
				t.pe=buscar(",");
				break;
		} else if (c == ':') {
				t.compLex = DOS_PUNTOS;
				t.pe=buscar(":");
				break;
		}
		else if(c == '"') {
			//es un identificador (o palabra reservada)
			i=0;
			do{
				lexema[i]=c;
				i++;
				c=fgetc(archivo);
				if (c==EOF)
					error("No se ha cerrado la cadena");
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamaño de buffer");
			}while( (c != '"') );
			if (c == '"') {
				lexema[i++]=c;
				lexema[i]='\0';
				strcpy(e.lexema,lexema);
				e.compLex=LITERAL_CADENA;
				insertar(e);
				t.pe=buscar(lexema);
				t.compLex=LITERAL_CADENA;
			} else  {
				error("Terminador de Identificador no encontrado... %c\n");
			}
			break;
		}
		else if (isalpha(c))
		{
			//es un identificador (o palabra reservada)
			i=0;
			do{
				lexema[i]=c;
				i++;
				c=fgetc(archivo);
				if (i>=TAMLEX)
					error("Longitud de Identificador excede tamaño de buffer");
			}while(isalpha(c) || isdigit(c));
			lexema[i]='\0';
			if (c!=EOF)
				ungetc(c,archivo);
			else
				c=0;
			t.pe=buscar(lexema);
			t.compLex=t.pe->compLex;
			if (t.pe->compLex==-1)
			{
				error("Caracter no encontrado");
				// strcpy(e.lexema,lexema);
				// e.compLex=LITERAL_CADENA;
				// insertar(e);
				// t.pe=buscar(lexema);
				// t.compLex=LITERAL_CADENA;
			}
			break;
		}
		else if (isdigit(c))
		{
				//es un numero
				i=0;
				estado=0;
				acepto=0;
				lexema[i]=c;
				
				while(!acepto)
				{
					switch(estado){
					case 0: //una secuencia netamente de digitos, puede ocurrir . o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=0;
						}
						else if(c=='.'){
							lexema[++i]=c;
							estado=1;
						}
						else if(tolower(c)=='e'){
							lexema[++i]=c;
							estado=3;
						}
						else{
							estado=6;
						}
						break;
					
					case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
						c=fgetc(archivo);						
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=2;
						}
						// else if(c=='.')
						// {
						// 	i--;
						// 	fseek(archivo,-1,SEEK_CUR);
						// 	estado=6;
						// }
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 2://la fraccion decimal, pueden seguir los digitos o e
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=2;
						}
						else if(tolower(c)=='e')
						{
							lexema[++i]=c;
							estado=3;
						}
						else
							estado=6;
						break;
					case 3://una e, puede seguir +, - o una secuencia de digitos
						c=fgetc(archivo);
						if (c=='+' || c=='-')
						{
							lexema[++i]=c;
							estado=4;
						}
						else if(isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 4://necesariamente debe venir por lo menos un digito
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							sprintf(msg,"No se esperaba '%c'",c);
							estado=-1;
						}
						break;
					case 5://una secuencia de digitos correspondiente al exponente
						c=fgetc(archivo);
						if (isdigit(c))
						{
							lexema[++i]=c;
							estado=5;
						}
						else{
							estado=6;
						}break;
					case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
						if (c!=EOF)
							ungetc(c,archivo);
						else
							c=0;
						lexema[++i]='\0';
						acepto=1;
						t.pe=buscar(lexema);
						if (t.pe->compLex==-1)
						{
							strcpy(e.lexema,lexema);
							e.compLex=LITERAL_NUM;
							insertar(e);
							t.pe=buscar(lexema);
						}
						t.compLex=LITERAL_NUM;
						break;
					case -1:
						if (c==EOF)
							error("No se esperaba el fin de archivo");
						else
							error(msg);
						exit(1);
					}
				}
			break;
		}
		else if (c!=EOF)
		{
			sprintf(msg,"%c no esperado",c);
			error(msg);
		}
	}
	if (c==EOF)
	{
		t.compLex=EOF2;
		// strcpy(e.lexema,"EOF");
		sprintf(e.lexema,"EOF");
		t.pe=&e;
	}
	
}

void match(int expToken)
{
	if (t.compLex == expToken)
    {
        getToken();
    }
    else
    {
		sintaxError = 1;
        sprintf(cad, "Se esperaba %s", getTokenFromCode(expToken));
        errorSyntax(cad);
		sincronizar(expToken);
    }
}

// END: Rutinas del analizador lexico


// BEGIN: Rutinas del analizador sintactico

void errorSyntax(const char* mensaje)
{
	printf("Lin %d: Error Sintactico. %s.\n", numLinea, mensaje);	
}

void json(int indent)
{
    element(indent, TRUE);
}

void element(int indent, int ignoreTag)
{
	if (t.compLex == L_LLAVE)
    {
        object(indent, ignoreTag);
    }
    else if (t.compLex == L_CORCHETE)
    {
        array(indent);
    }
    else
    {
        errorSyntax("Elemento JSON no válido");
		//sincronizar(L_LLAVE);  // Sincronización en el modo de pánico
    }
}

void array(int indent)
{
	match(L_CORCHETE);
    arrayB(indent);
}

void arrayB(int indent)
{
	if (t.compLex == R_CORCHETE)
    {
        match(R_CORCHETE);
    }
    else if (t.compLex == L_CORCHETE || t.compLex == L_LLAVE)
    {
        element_list(indent + 1);
        match(R_CORCHETE);
    }
    else
    {
        errorSyntax("Array no válido");
    }
}

void element_list(int indent)
{
	element(indent, FALSE);
	element_listB(indent);
}

void element_listB(int indent)
{
	if (t.compLex == COMA)
    {
        match(COMA);
        element(indent, FALSE);
        element_listB(indent);
    }
}

void object(int indent, int ignoreTag)
{
    match(L_LLAVE);
	printIndented(indent);
	if(!ignoreTag) printf("<item>\n");
    objectB(indent, ignoreTag);
}

void objectB(int indent, int ignoreTag)
{
	if (t.compLex == R_LLAVE)
    {
        match(R_LLAVE);
		printIndented(indent);
    	if(!ignoreTag) printf("</item>\n");
    }
    else
    {
        attributes_list(indent + 1);
		match(R_LLAVE);
		printIndented(indent);
    	if(!ignoreTag) printf("</item>\n");
    }
}

void attributes_list(int indent)
{
    attribute(indent);
    attributes_listB(indent);
}

void attributes_listB(int indent)
{
	if (t.compLex == COMA)
	{
		match(COMA);
        attribute(indent);
        attributes_listB(indent);
	}
}

void attribute(int indent)
{
	char* endLex = attribute_name(indent);
    match(DOS_PUNTOS);
    attribute_value(indent, endLex);
}

char* attribute_name(int indent)
{
	char* lexema = t.pe->lexema;
	if (t.compLex == LITERAL_CADENA)
    {
		printIndented(indent);
		printf("<%s>", lexema);
        match(LITERAL_CADENA);
		return lexema;
    }
    else
    {
        errorSyntax("Nombre de atributo no válido");
		//sincronizar(LITERAL_CADENA);  // Sincronización en el modo de pánico
		return NULL;  // No se puede obtener el nombre de atributo en este caso
    }
}

void attribute_value(int indent, char* endLex)
{
	if (t.compLex == LITERAL_CADENA || t.compLex == LITERAL_NUM || t.compLex == PR_TRUE || t.compLex == PR_FALSE || t.compLex == PR_NULL)
    {
        printToken(t);
		match(t.compLex);
		printf("</%s>\n", endLex);
    }
    else if (t.compLex == L_LLAVE)
    {
        object(indent + 1, FALSE);
		printIndented(indent);
        printf("</%s>\n", endLex);
    }
    else if (t.compLex == L_CORCHETE)
    {
        array(indent + 1);
		printIndented(indent);
        printf("</%s>\n", endLex);
    }
    else
    {
        errorSyntax("Valor de atributo no válido");
		//sincronizar(L_LLAVE);  // Sincronización en el modo de pánico
    }
}

void sincronizar(int expToken)
{
	// Modo Pánico: continuar hasta encontrar un punto de sincronización
    while (t.compLex != expToken && t.compLex != EOF2)
    {
        getToken();
    }
    if (t.compLex == expToken)
    {
        getToken();
    }
}

// END: Rutinas del analizador sintactico
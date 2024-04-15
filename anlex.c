/*
 *	Analizador L�xico	
 *	Curso: Compiladores y Lenguajes de Bajo de Nivel
 *	Pr�ctica de Programaci�n Nro. 1
 *	
 *	Descripcion:
 *	Implementa un analizador l�xico que reconoce n�meros, identificadores, 
 * 	palabras reservadas, operadores y signos de puntuaci�n para un lenguaje
 * 	con sintaxis tipo Pascal.
 *	
 */

/*********** Inclusi�n de cabecera **************/
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
int numLinea=1;			// Numero de Linea

/**************** Funciones **********************/


// Rutinas del analizador lexico

void error(const char* mensaje)
{
	printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);	
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
					error("Longitud de Identificador excede tama�o de buffer");
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
					error("Longitud de Identificador excede tama�o de buffer");
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

int main(int argc,char* args[])
{
	int beforeLine = 0;
	// inicializar analizador lexico
	initTabla();
	initTablaSimbolos();

	if(argc > 1)
	{
		if (!(archivo=fopen(args[1],"rt")))
		{
			printf("Archivo no encontrado.\n");
			exit(1);
		}

		while (t.compLex!=EOF2){
			getToken();
			if(beforeLine == 0) {
				beforeLine = numLinea;
				printf("Lin %3d: %s",numLinea,getTokenFromCode(t.compLex));
			} else if(beforeLine != numLinea ) {
				beforeLine = numLinea;
				printf("\nLin %3d: %s",numLinea,getTokenFromCode(t.compLex));
			} else {
				printf("\t%s",getTokenFromCode(t.compLex));
			}
		}
		fclose(archivo);
	}else{
		printf("Debe pasar como parametro el path al archivo fuente.\n");
		exit(1);
	}

	return 0;
}

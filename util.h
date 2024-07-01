#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Función para eliminar las comillas de un string
// Liberar memoria luego de utilizar el string retornado
char* remove_quotes(const char* str) {
    size_t len = strlen(str);

    // Si la longitud es menor o igual a 2, no puede haber comillas para eliminar
    if (str == NULL || len <= 2) {
        return strdup("");
    }
    
    // Si la cadena no comienza con comillas retornar la cadena
    if(str[0] != '"') {
        return strdup(str);
    }

    // Crear un nuevo string sin las comillas
    char* result = (char*)malloc((len - 1) * sizeof(char));
    if (result == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(EXIT_FAILURE);
    }

    // Copiar el contenido sin las comillas
    strncpy(result, str + 1, len - 2);
    result[len - 2] = '\0';

    return result;
}


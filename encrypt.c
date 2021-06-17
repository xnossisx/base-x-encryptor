#include "encrypt.h"
#include <stdio.h>

lengthString encode(char *data, int *inputFormat, int *outputFormat, int datalen, int inputFormatLen, int outputFormatLen, bool verbose)
{
    VariableInt *dataVarInteger = malloc(sizeof(VariableInt));
    *dataVarInteger = variableInt();
    // Basic plan: Convert from base (inputFormatSize) to base (outputFormatSize), they shouldn't share any traits
    // Convert the data from the open file into the VariableInt
    {
        VariableInt multer = variableInt();
        setVarInt(&multer, inputFormatLen);
        for (int i = 0; i < datalen; i++)
        {
            int reformat = inputFormat[(unsigned char)data[i]];
            if (reformat == -1)
            {
                fprintf(stderr, "\033[31mError: The output format was insufficient to decrypt this file.\n\033[0m");
                exit(EXIT_FAILURE);
            }
            multVarInt(dataVarInteger, &multer, dataVarInteger);
            addVarIntIP(dataVarInteger, (unsigned char)reformat); // Insert the character here. What could possibly go wrong?
        }
        destroyVarInt(multer);
    }
    // Read the VariableInt back out, in a different base encrypted in the output symbol format
    {
        int blocklen = (bitlenVarInt(dataVarInteger) / ((outputFormatLen == 2) ? 2 : 1));
        int pos = blocklen - 1;
        char *newStr = malloc(blocklen); // This will be the maximum size needed to store the base conversion
        VariableInt outputLen = variableInt();
        outputLen.data[0] = outputFormatLen;
        while (dataVarInteger->data[0] != 0 || dataVarInteger->len > 1)
        {
            newStr[pos - 1] = outputFormat[(unsigned char)divVarInt(dataVarInteger, &outputLen, dataVarInteger)];
            pos--;
        }
        destroyVarInt(outputLen);
        destroyVarInt(*dataVarInteger);
        free(dataVarInteger);
        
        char *ret = malloc((blocklen - pos ) * sizeof(char));
        BLOCKDATA i = 0;
        for (; i < blocklen - pos; i++)
        {
            ret[i] = newStr[i + pos];
        }
        free(newStr);
        return ((lengthString) {ret, i - 1});
    }
}
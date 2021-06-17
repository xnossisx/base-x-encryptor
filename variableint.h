// This section works with integers of variable length, and basic operations needed for base conversion are included.
#define VARINT
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

struct VariableInt
{
    uint64_t *data;
    int len;
};
typedef struct VariableInt VariableInt;

VariableInt variableInt();
void destroyVarInt(VariableInt vi);
VariableInt variableIntL(unsigned int len);
bool eqVarInt(VariableInt *vi1, VariableInt *vi2);
void setVarInt(VariableInt *vout, uint64_t set);
void addVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout);
void addVarIntIP(VariableInt *vi, uint64_t ins);
void subVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout);
void multVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout);
uint64_t divVarInt(VariableInt *vi1, VariableInt *vi2, VariableInt *vout);
uint64_t modVarIntI(VariableInt *vi1, uint64_t modand);
void lshiftVarInt(VariableInt *vi, VariableInt *vout, unsigned int ls);

//Other helper functions
int maxVL(VariableInt *vi1, VariableInt *vi2);
int minVL(VariableInt *vi1, VariableInt *vi2);
int bitlenVarInt(const VariableInt *vi);
void printVarInt(VariableInt *vi);


#ifndef min
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
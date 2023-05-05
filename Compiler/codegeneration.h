int InitCodeGeneration(char* filename);
void EmitCode(const char *format, ...);
void EmitPushConstant(int value);
void EmitPopPointer(int index);
void EmitPopTemp(int index);
void EmitConstructor(char* name, int localFieldsCount);
void EmitFunction(Symbol* symbol);
void EmitCall1(Symbol* symbol);
void EmitCall2(char* className, char* functionName, int argumentCount);
void EmitString(char* string);
void EmitReturn(Symbol* symbol);
void StopCodeGeneration();
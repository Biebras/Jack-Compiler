int InitCodeGeneration(char* filename);
void EmitCode(const char *format, ...);
void EmitPushConstant(int value);
void EmitPopTemp(int index);
void EmitFunction(char* name, int localVariablesCount);
void EmitCall(char* name, int localVariablesCount);
void EmitString(char* string);
void StopCodeGeneration();
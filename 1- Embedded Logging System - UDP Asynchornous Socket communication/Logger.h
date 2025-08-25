
enum LOG_LEVEL {
	DEBUG = 1,
	WARNING = 2,
	ERROR = 3,
	CRITICAL = 4
};

int InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char* prog, const char* func, int line, const char* message);
void ExitLog();





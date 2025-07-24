#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*function_ptr) (void);

int main()
{
	void *handle;
	handle = dlopen("./libcontats.so", RTLD_LAZY);
	if(!handle)
	{
		fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
		exit(1);	
	}	

	function_ptr menu = (function_ptr)dlsym(handle, "menu");

	if (!menu)
		fprintf(stderr, "Ошибка получения функции: %s\n", dlerror());

	menu();

	dlclose(handle);

	return 0;
}

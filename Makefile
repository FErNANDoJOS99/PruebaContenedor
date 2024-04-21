SHELL=/bin/bash



compilar:hoy-no-circula.c
		@echo "estoy compilando el programa"
		cc -o hoy  hoy-no-circula.c
		./hoy 2024-01-01  < entrada.tsv


otroEjemplo:hoy-no-circula.c
		@echo "estoy compilando el programa "
		cc -o hoy  hoy-no-circula.c
		./hoy 2024-01-07  < entrada.tsv

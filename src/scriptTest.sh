./argus -e "grep -v ^# /etc/passwd | cut -f7 -d: /etc/passwd | uniq | wc -l" # mostrar pipes
./argus -e "cat Resources/BeeMovie.txt" # mostrar cat de grandes ficheiros (output)
./argus -m "2" 			# mostrar tempo de execução
./argus -e "sleep 10"	# mostrar tempo de execução
./argus -m "-1"
./argus -i "1"					# mostrar tempo de inatividade
./argus -e "sleep 10 | wc -l"	# mostrar tempo de inatividade
./argus -e "sleep 40"	# pode ser usado para mostrar o -t

#linha 212, 240 e 355
# "&& isdigit()..." -> foi removido porque além de não ser preciso, 
# estava a causar com que os processos não fossem mortos 
# visto que essa função vai receber um int em vez de um char
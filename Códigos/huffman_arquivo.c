/*
 Huffman coding and decoding

 A codificação Huffman é um dos algoritmos mais simples para compactar dados. Embora, seja
 muito antigo e simples, ainda é amplamente utilizado (por exemplo: em alguns estágios de JPEG,
 MPEG, etc.).

 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//Quantidade de caracteres possiveis
#define NUM_CHARS 256

typedef struct _treenode treenode;
struct _treenode{
	int	freq;//frequencia
	unsigned char ch;//caractere
	treenode *esquerda,//filho a esquerda da arvore huffman
			*direita;//filho a direita da arvore huffman
};

//fila de prioridade implementada como heap binaria
typedef struct _pq{
	int	heap_size;
	treenode *A[NUM_CHARS];
} PQ;

//fila vazia
void create_pq(PQ *p){
	p->heap_size=0;
}

//pai do nodo da heap
int pai(int i){
	return (i-1) / 2;
}

//filho esquerdo do nodo da heap

int esquerda(int i){
	return i * 2 + 1;
}

//filho direito do nodo da heap
int direita(int i){
	return i * 2 + 2;
}

//fazemos a subheap com raiz(root) i uma heap, assumindo que esquerda(i) e direita(i) sao heaps
void heapify(PQ *p, int i){
	int		e, d, menor;
	treenode	*t;
	e = esquerda(i);
	d = direita(i);
	//acha o menor valor de pai, esquerda e direita
	if (e < p->heap_size && p->A[e]->freq < p->A[i]->freq)
		menor = e;
	else
		menor = i;
	if (d < p->heap_size && p->A[d]->freq < p->A[menor]->freq)
		menor = d;
	//caso necessario, troca pai com menor
	if (menor != i){
		t = p->A[i];
		p->A[i] = p->A[menor];
		p->A[menor] = t;
		heapify(p, menor);
	}
}

/*insere um elemento na fila de prioridade, r->freq sendo a prioridade
onde r é raiz e freq a frequencia*/
void insert_pq(PQ *p, treenode *r){
	int	i;

	p->heap_size++;
	i = p->heap_size-1;
	while ((i > 0) && (p->A[pai(i)]->freq > r->freq)){
		p->A[i] = p->A[pai(i)];
		i = pai(i);
	}
	p->A[i]=r;
}

//remove o elemento na frente/no topo da fila (aquele com a menor frequencia)
treenode *extract_min_pq(PQ *p){
	treenode *r;
	if (p->heap_size==0){
		printf("heap tá dando underflow maluco, olha isso aí!\n");
		exit(1);
	}
	//pega valor de retorno da raiz
	r = p->A[0];
	//pega o ultimo valor e coloca na raiz, assim como em heapsort
	p->A[0] = p->A[p->heap_size-1];
	p->heap_size--;
	//como esquerda e direita sao heap, fazemos a raiz uma heap
	heapify(p, 0);
	return r;
}

//le o arquivo, da a frequencia de cada caractere e os coloca em um vetor
unsigned int get_frequencias(FILE *f, unsigned int v[]){
	int	r, n;
	for (n=0;;n++){
		//o fgetc aqui pega um char unsigned e o converte em int
		r = fgetc(f);
		//se nao tem mais o que percorrer, break para sair do loop
		if (feof(f)) break;
		//mais um caractere lidoc aso n saia do loop claro
		v[r]++;
	}
	return n;
}

//Algoritmo de Huffman, faz a arvore de huffman das frequencias no vetor freq[]
treenode *build_huffman(unsigned int freqs[]) {
	int	i, n;
	treenode *x, *y, *z;
	PQ	p;

	//começamos criando uma fila vazia
	create_pq(&p);

	//para cada caractere, fazemos uma heap ou nodo da arvore com seu valor e frequencia
	for(i=0; i<NUM_CHARS; i++) {
		x = malloc (sizeof (treenode));
		//folha da arvore
		x->esquerda=NULL;
		x->direita=NULL;
		x->freq=freqs[i];
		x->ch=(char) i;
		//insere o nodo na heap
		insert_pq(&p, x);
	}
	//até aqui temos a heap como uma arvore de filhos unicos
	n = p.heap_size-1;
	/*Aqui nos colocamos heapsize-1 dentro de n pois heapsize NAO eh invariante em loop,
	 assim, caso seja inseridas duas coisas e removermos uma de cada vez, no fim de heapsize-1
	 iteracoes vai ficar uma arvore restando na heap*/
	for(i=0; i<n; i++){

		//criando um novo nodo chamado z dos dois nodos com menor frequencia x e y
		z = malloc(sizeof(treenode));
		x = extract_min_pq(&p);
		y = extract_min_pq(&p);
		z->esquerda=x;
		z->direita=y;
		//a frequencia de z sera a soma de x e y
		z->freq = x->freq + y->freq;
		//colocamos de volta na fila
		insert_pq (&p, z);
	}

	//fazemos retorno da ultima coisa na fila a arvore huffman

	return extract_min_pq (&p);
}

/*passamos pela arvore huffman construindo assim o codigo para cada
caractere e guardamos em um vetor chamado codes[]*/
void traverse (treenode *r,	int level, char code_so_far[], char *codes[]) {

	//se estiver em nodo folha
	if ((r->esquerda == NULL) && (r->direita == NULL)) {
		code_so_far[level] = 0;
		codes[r->ch] = strdup (code_so_far);
	}else{
		//caso nao seja nodo folha, ir para esquerda com bit 0
		code_so_far[level] = '0';
		traverse (r->esquerda, level+1, code_so_far, codes);

		//ir para direita com bit 1
		code_so_far[level] = '1';
		traverse (r->direita, level+1, code_so_far, codes);
	}
}

int nbits, current_byte, nbytes;

//output de um bit único para um arquivo aberto
void bitout (FILE *f, char b) {
	//faz um shift a esquerda no byte atual em 1
	current_byte <<= 1;
	//iremos colocar um 1 no final do byte atual caso b seja 1
	if (b == '1') current_byte |= 1;
	//mais um bit
	nbits++;
	//se tiver bits suficientes, ou seja 8, escrever o byte
	if (nbits == 8) {
		fputc (current_byte, f);
		nbytes++;
		nbits = 0;
		current_byte = 0;
	}
}

//agora utilizamos os codigos no vetor criado para codificar o arquivo
void encode_file (FILE *infile, FILE *outfile, char *codes[]){
	unsigned char ch;
	char *s;
	//inicializamos umas variaveis globais para serem utilizadas na funcao bitout
	current_byte = 0;
	nbits = 0;
	nbytes = 0;

	//continua ate o fim do arquivo
	for (;;){
		//pega um char
		ch = fgetc(infile);
		if (feof(infile)) break;
		//coloca a bitstring correspondente na outfile
		for (s=codes[ch]; *s; s++)bitout(outfile, *s);
	}
	//termina o ultimo byte
	while (nbits)bitout(outfile, '0');
}

int main (int argc, char *argv[]){
	FILE *f, *g;
	treenode *r;//raiz
	unsigned int n,//numero de bytes
			freqs[NUM_CHARS];//frequencia dos caracteres
	char *codes[NUM_CHARS],//vetor de codigos, um por caractere
		code[NUM_CHARS],//apenas um vetor para carregar um codigo
		fname[100];//nome do arquivo de output

	//setando todas as frequencias em 0
	memset (freqs, 0, sizeof (freqs));

	f = fopen ("teste.txt", "r");
	if (!f) {
		perror ("teste.txt");
		exit (1);
	}
	//pegamos as frequencias do arquivo
	n = get_frequencias(f, freqs);
	fclose (f);
	//criamos a arvore huffman
	r = build_huffman(freqs);
	//passamos na arvore colocando os codigos em seu vetor respectivo
	traverse(r, 0, code, codes);
	//nomeamos o arquivo de saida
	sprintf(fname, "%s.huf", "teste.txt");
	g = fopen(fname, "w");
	if (!g) {
		perror(fname);
		exit (1);
	}
	//escrevemos as frequencias no arquivo
	fwrite(freqs, NUM_CHARS, sizeof (int), g);
	//escrevendo o numero de caracteres no arquivo como int binario
	fwrite(&n, 1, sizeof (int), g);
	//abre o arquivo de input de novo
	f = fopen("teste.txt", "r");
	if (!f) {
		perror("teste.txt");
		exit (1);
	}
	//codifica f para g com codigos
	encode_file(f, g, codes);
	fclose (f);
	fclose (g);

	printf("%s is %0.2f%% of %s\n",
		fname, (float) nbytes / (float) n, "teste.txt");
	exit(0);
}

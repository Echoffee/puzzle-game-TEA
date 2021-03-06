#include <stdio.h>
#include <stdlib.h>
#include <game.h>
#include <utility.h>

#define TAILLE_PLATEAU_RH 6

struct game_s{
	int width;
	int height;
	piece *pieces;
	int	nb_moves;
	int nb_pieces;
};

bool estPositionValide(game g, piece p);

game new_game (int width, int height, int nb_pieces, piece *pieces){
	//les allocations mémoires du game et de son tableau de piece...
	game new_game = (game)malloc(sizeof(struct game_s));

	if(new_game == NULL)
		error("new_game(), probleme d allocation memoire");

	new_game -> pieces = (piece*) malloc(sizeof(piece) * nb_pieces);
	
	if(!new_game -> pieces)
		error("Allocation new_game -> pieces");

	//... Affectation des valeurs par copie
	new_game -> nb_pieces = nb_pieces;
	for (int i = 0; i < nb_pieces; i++)
	{
		new_game -> pieces[i] = new_piece( 0, 0, 0, 0, false,  false);

		if (!new_game -> pieces[i])
			error("Allocation new_game -> pieces[i]");

		copy_piece(pieces[i], new_game -> pieces[i]);

		if (!new_game -> pieces[i])
		{
			printf("i=%d\n", i);
			error("Copie de pièce");
		}
	}

	new_game -> width = abs(width);
	new_game -> height = abs(height);
	new_game -> nb_moves = 0;

	return new_game;
}

void delete_game (game g)
{
	if(g != NULL)
	{
		int nb_pieces = game_nb_pieces(g);
		//On libère d'abord le tableau des pieces
		for (int i = 0; i < nb_pieces; i++)
		{
			delete_piece(g -> pieces[i]);
		}
		free(g -> pieces);
		//...Ensuite le reste de la structure
		free(g);
	}
}

void copy_game(cgame src, game dst)
{
	if(src == NULL || dst == NULL)
		error("copy_game, src ou dst ne sont pas alloué");

	//On supprime les anciennes pieces
	for (int i = 0; i < dst -> nb_pieces; i++)
		delete_piece(dst -> pieces[i]);
	free(dst -> pieces);

	//D'abord les propriétés directes
	dst -> nb_pieces = game_nb_pieces(src);
	dst -> nb_moves = game_nb_moves(src);
	dst -> width = game_width(src);
	dst -> height = game_height(src);

	//...Ensuite le tableau des pièces
	//On réalloue du tableau des pieces
	dst -> pieces = (piece*) malloc(sizeof(piece) * dst -> nb_pieces);

	if (!dst -> pieces)
		error("Echec du deuxième malloc");

	for (int i = 0; i < src -> nb_pieces; i++)
	{
		dst -> pieces[i] = new_piece(0, 0, 0, 0, false, false);

		if (!dst -> pieces[i])
			error("Allocation dst -> pieces[i]");

		copy_piece(src -> pieces[i], dst -> pieces[i]);
	}
}

bool play_move(game g, int piece_num, dir d, int distance)
{
	//Variable pour la vérification du parcours (distance à parcourir sur les axes)
	int dist_dir = (d == DOWN ||d == LEFT) ? -1 * distance : distance ;
    //On vérifie qu'il n'entre au contact d'aucune pièce et qu'il reste sur le plateau en utilisant un clone-cobaye (ptest)
	//piece ptest = new_piece_rh(0, 0, true, true);
	piece ptest = new_piece(0, 0, 0, 0, true, true);
	copy_piece(g -> pieces[piece_num], ptest);

	int ptestx = get_x(ptest);
	int ptesty = get_y(ptest);
	if (((d == UP || d == DOWN) && !can_move_y(game_piece(g, piece_num))) || ((d == LEFT || d == RIGHT) && !can_move_x(game_piece(g, piece_num))))
	{
		delete_piece(ptest);
		return false;
	}

	//Cette première boucle permet de faire pas à pas le déplacement lorsque celui ci est supérieur à 1
	for (int step = 0; step < abs(distance); step++)
	{
		move_piece(ptest,d,1);
		if(!estPositionValide(g, ptest))
		{
			delete_piece(ptest);
			return false;
		}

		//Boucle qui vérifie la colision avec les autres pieces
		for (int i = 0; i < game_nb_pieces(g); i++)
		{
        	// s'il ne s'agit pas de la même pièce, alors on regarde s'il y a contact.
			if (i != piece_num && intersect(ptest, g -> pieces[i]))
			{
				delete_piece(ptest);
				return false;
			}
		}	
	}
	// Si ptest n'a pas bougé, c'est que move_piece a trouvé que le mouvement sur distance n'était pas bon.
	if (distance != 0 && ((can_move_y(ptest) && (d == UP || d == DOWN) && get_y(ptest) != ptesty + dist_dir) || (can_move_x(ptest) && (d == LEFT || d == RIGHT) && get_x(ptest) != ptestx + dist_dir))) {
		delete_piece(ptest);
		return false;
	}

	delete_piece(ptest);
    //Si tout est bon, alors on lance l'algorithme de déplacement de la pièce, puis on ajoute les moves correspondant.
	move_piece(g -> pieces[piece_num], d, abs(distance));	
	g -> nb_moves += abs(distance);
	return true;
}

//Vérifie si la position de la piece est bien dans le plateau
bool estPositionValide(game g, piece p){
 	if((get_x(p) < 0) || (get_y(p) < 0))
 		return false;

	if(get_x(p) + get_width(p) > game_width(g) || get_y(p) + get_height(p) > game_height(g) )
		return false;
 
 	return true;
 }

// --------- Fonctions Simples ----------------
int game_square_piece (game g, int x, int y){
	
	int** tab = mapPieces(g->pieces, game_nb_pieces(g), game_width(g), game_height(g));
	int value = tab[x][y];
	free(tab[0]);
	free(tab);
	return value;
}

//retourne le nombre de move fait
int game_nb_moves(cgame g)
{
	if(g != NULL)
    	return g -> nb_moves;
    
    return -1;
}

int game_nb_pieces(cgame g)
{
	if (!g)
		error("Allocation cgame game_nb_pieces");

	return g -> nb_pieces;
}
int game_width(cgame g){
	if(g == NULL)
		error("game_width(), g n'est pas alloue");
	return g -> width;
}

int game_height(cgame g){
	if(g == NULL)
		error("game_height(), g n'est pas alloue");
	return g -> height;
}

cpiece game_piece(cgame g, int piece_num)
{
	//Vérifie que l'indice piece_num pièce est bien dans le jeu
    if (piece_num < 0 || piece_num > game_nb_pieces(g) - 1)
		error("L'index de la pièce recherchée est impossible (trop grand ou négatif)");

	//Si elle existe, on la renvoie
	return g -> pieces[piece_num];
}

//error est une fonction qui permet d'envoyer un message sur stderr et de faire un exit(EXIT_FAILURE). 
//Elle permet d'éviter de recopier a chaque fois 2 à 3 lignes.
void error(char* s)
{
	fprintf(stderr, "Fatal Error : %s.\n", s);
	exit(EXIT_FAILURE);
}

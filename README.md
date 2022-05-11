L'archive sans le fichier mp3 pèse 44.6 kB elle pèse 4.6 mb avec le fichier mp3

Les deux fichiers image sont encodés dans le fichier mp3 pour lancer le code le fichier mp3 doit être dans le dossier du projet.

how to use:

    make clean all && ./Solar_System

S'il n'y aucun fichier image dans le dossier, alors le programme tente de décoder le fichier mp3
si cela echoue, c'est que le fichier mp3 fournis dans l'archive a été remplacer ou corrompu.

Dans le cas contraire, le code s'exécute normalement en utilisant les fichiers image présents.

À savoir une fois les fichiers image décodés à partir du fichier mp3, ces donnes sont supprimer du fichier mp3,
mais également que l'intensité de la lumière augmente au fils du temps pour un plus bel effet si vous ne voulez pas de cette augmentation, 
il suffit de commenter la ligne 325.

    intensite += 0.001;

Crédits :

Musique : Sacred Breath from Johann Kotze , soundcloud link : https://soundcloud.com/johannkotzemusicyoga/sacred-breath

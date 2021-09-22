# La tête dans les étoiles

Le répertoire `fits/` contient des images captées par des outils
astronomiques. Ces images sont des matrices de pixels dont les valeurs
sont codées sur 16 bits. Afin de visualiser correctement ces images, il
convient de transformer ces valeurs. La transformation la plus simple
est une mise à l'échelle qui consiste à transformer linéairement la
plage des valeurs de l'image en la plage 0..255.

Le code source `linear.cpp` propose un code à paralléliser pour
effectuer cette transformation.

Le fichier `Makefile` permet de compiler le programme. Pour cela, il
convient d'indiquer le nom de votre compilateur C++ OpenMP sur la ligne
`CXX` du fichier puis d'utiliser la commande `make` :

```
$ make linear
```

Le programme `linear` est à invoquer avec deux paramètres, l'image à
transformer et le nombre d'itérations à faire. Il produit en sortie le
temps moyen écoulé en secondes pendant la partie calcul ainsi qu'un
fichier `/tmp/resca.pgm` qui contient l'image transformée.

``` 
$ ./linear fits/eso137_optical_R.fits 1
1.65753
$ xdg-open /tmp/resca.pgm
```

Le script `bench` permet de tester différents paramètres (nombre de
threads, etc) et de générer un fichier `stats.csv` qui recense les temps
moyen. Il est possible de modifier le comportement de ce script en
affectant certaines variables d'environnement.

```
$ ITERATE=5 IMAGE=fits/lmc.fit ./bench
```

Le script `show` permet d'afficher ces statistiques sous forme de
courbes si toutefois vous disposez de `python3` et des bibliothèques
`pandas` et `matplotlib`. Le code source se trouve dans le répertoire
`scripts/`.

```
$ ./show
```

Enfin, le répertoire `include` contient la bibliothèque `CImg.h`
utilisée pour manipuler des images ainsi qu'un plugin rudimentaire
`fits.h` pour importer les images FITS 16 bits.


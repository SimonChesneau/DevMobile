<h1>Mise a jours du code donné:</h1>
<h2>Renforcement du code</h2>
<h3>Problème</h3>
<p>Lors de l'installation de l'APK sur le téléphone de mathias, un bug jusque là jamais apparus s'est mis a survenir de manière aléatoire. Après avoir investigué le sujet,
        nous nous sommes rendu compte que certains téléphone alloue des zone de mémoire normalement interdire, engendrant la non réation d'objets C++ et donc des nullpointers par la
        suite.</p>
<h3>Résolution</h3>
<p>Afin de rendre le code plus robuste et consistant, nous avons identifié les endroits sur lesquels cette erreure se produisait.
Heureusement pour nous, cela ne se produisait que lors de l'initialisation de l'encodeur. Nous avons donc ajot une boucle initialisant l'encodeur 
tant que celui-ci n'était pas créer entièrement. </p>
<h4>Code précédent</h4>
<code>       
m_Encode = new Encoder();
m_Encode->setSocketClientH264(m_socket);
m_Encode->InitCodec(400, 608, 15, 20000); //480, 640, 15, 100000
test = m_Encode->getStatus();
</code>
<h4>Code mis à jours pour plus de robustesse</h4>
<code>
do {
        m_Encode = new Encoder();
        m_Encode->setSocketClientH264(m_socket);
        m_Encode->InitCodec(400, 608, 15, 20000); //480, 640, 15, 100000
        test = m_Encode->getStatus();
}while(test != AMEDIA_OK);
        
</code>
<p>De cette manière, le statut de l'encodeur doit forcément être AMEDIA_OK pour pouvoir passer à la suite.</p>
<p>Cette partie nous a semblé essentiel, malgrés le besoins de cette evaluation, car cela permet a tout portable android de passer cette erreur, et non plus que les plus chanceux.</p>

<h3>Ajout de la récupération automatique de la taille de l'image dans l'initialisation de l'encodeur</h3>
<p>Jusqu'ici, nous donnions manuelement les dimensions de l'image à l'encodeur afin qu'il puisse encoder l'encoder. Nous avons décider de rendre cette partie automatique
pour éviter des erreur de segmentation lors du <code>memcpy</code> effectuer sur les buffers images. </p>
<h4>Code précédent </h4>
<p>précédemment nous utilisions le <code>SetUpEncoder</code> pour spécifier les dimensions des images</p>
<code>
m_Encode->InitCodec(400, 608, 15, 20000);
</code>
<h4>Mis à jours du code pour plus de maléabilité</h4>
<p>Lors de la récupération des images, nous envoyons les dimanesions au serveur lors du premier envoi.
C'est a ce moment là que nous initialisons les bonnes dimensions d'image à l'encodeur afin qu'il puisse encoder les images sans créer d'erreurs.</p>
<code>
do {
m_Encode->InitCodec(display_mat.rows, display_mat.cols, 15,
                    20000);

m_retrievedImageWidth = display_mat.cols;
m_retrievedImageHeight = display_mat.rows;

}while(m_Encode->getStatus() != AMEDIA_OK);
</code>
<p>Cette partie nous a semblé essentiel, de la même manière que la première, car elle permet de pouvoir lancer le programme sur n'importe quel portable, en dépit des 
dimensions d'images capturées</p>

<h2>Ajout de nouvelles fonctionnalités</h2>
<p>Le sujet de cette évaluation était d'ajouter de nouvelles fonctionnalités a l'application. Nous avons donc créer une détection de visages via 2 moyens.</p>
<p>La raison pour laquelle nous avons créé 2 manière de détecter les visages plutot que d'ajouter plusieurs fonctionnalités est simple: nous avons essayer dans un premier temps d'ajouter
la détection de visages via openCV, mais après 2 jours d'essais infructueux, nous avons essayer la méthode par IA. A la suite de l'implémentation de la méthode de détection de visage par IA,
nous avons réussi a implémenter la détéction de visages via openCV. Afin de ne pas gacher ces nombreuses heures a chercher comment implémenter ces détections de visages, nous avons choisis de 
les implémenter toutes les 2 dans la version finale, avec un moyen de les activer et désactiver.</p>

<h3>Détéction de visage via une IA</h3>
<p>La détéction de visage via une IA à été la première implémentation à avoir marchée. Cette implémentation, faite via le tutorial présent sur <a href="https://medium.com/analytics-vidhya/building-a-face-detector-with-opencv-in-c-8814cd374ea1">ce lien</a>.</p>

<h4>Implémentation</h4>
<p>Lors du lancement, il est possible de séléctionner l'option de détection de visage en utilisant de l'IA:</p>
![alt text](https://github.com/SimonChesneau/DevMobile/blob/main/435171268_1911860759267412_1220162095142595644_1.jpg.jpg)


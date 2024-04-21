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
<h4>Code mis a jours pour plus de robustesse</h4>
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

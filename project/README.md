Projekat iz racunarske grafike

Oblasti koje su implementirane:
1. Prvih 8 nedelja zajedno sa face culling-om i depth testingom
2. Discard blending - za iscrtavanje drveca (decoration fs i vs)
3. Normal mapping - iscrtavanje kuce (house fs i vs)
4. Parallax mapping - iscrtavanje ravni i putanje do kuce (plane fs i vs)
5. Cubemaps - iscrtavanje dva skybox-a (skybox fs i vs)
6. U svim sejderima je implementirano svetlo po Blinn-Phong-ovom modelu

Projekat sadrzi i ImGui koji se pali pritiskom na dugle F1:
1. moguce citati podatke o kameri i otkljucati/zakljucati kameru
2. putem checkbox-a se moze menjati da li je dan ili noc
    -dan: svi sejderi koriste direkciono svetlo
    -noc: sejderi koriste dva point svetla smestana u lampama
3. moguce je menjati vektor pravca direkcionog svetla i svaku od njegovih komponenata
4. moguce je menjati komponente point svetala


F1-za paljenje ImGui prozora
Checkbox day - za biranje dana/noci na sceni
WASD - kretanje kamere
ColorEdit, DragFloat - za podesavanje komponenti svetala

youtube link:
https://www.youtube.com/watch?v=cfxO8ZMEcRg

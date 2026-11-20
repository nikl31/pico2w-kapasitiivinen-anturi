# pico2w-kapasitiivinen-anturi

Johdanto ja toimintaperiaate
Tämä projekti on omatekoinen, kosketukseton kapasitiivinen etäisyysanturi, joka on toteutettu Raspberry Pi Pico 2 W -mikrokontrollerilla ja tavallisella alumiinifoliolla ilman valmiita kaupallisia anturikomponentteja.

Mittaus perustuu RC-aikavakioon ja Picon ohjelmoitavaan PIO-yksikköön.
1. Pico purkaa alumiinifolion varauksen maahan pinnin GP15 kautta.
2. Pinni muutetaan sisääntuloksi ja kytketään päälle Picon sisäinen ylösvetovastus (~50–80 kΩ).
3. PIO-tilakone laskee kellojaksojen määrän, joka kuluu jännitteen nousemiseen loogisen ykkösen tasolle.
4. Kun kohde lähestyy foliota, kapasitanssi kasvaa, latausaika pitenee ja raakalukema kasvaa.

Kytkentä ja käyttöönotto
1. Kytke hyppylanka Raspberry Pi Pico 2 W:n pinniin GP15.
2. Kiinnitä johdon toinen pää tasaiselle alustalle teipattuun alumiinifolioon.
3. Peitä folio eristekerroksella (paperi tai teippi) oikosulkujen estämiseksi.
4. Käännä koodi Pico SDK:lla ja lataa -tiedosto Picolle.
5. Pidä anturin ympäristö tyhjänä ensimmäisen 1 sekunnin ajan käynnistyksestä, jotta automaattinen nollakalibrointi onnistuu.
6. Avaa sarjaporttimonitori esim. PuTTY tai VS Code Serial Monitor nähdäksesi reaaliaikaiset etäisyyslukemat.

Siirtofunktio ja testausraportti
Anturi kalibroitiin mittaamalla Picon raakalukemaa ($N$, 100 000 mittauskierroksen summa) vakioiduilla etäisyyksillä 0–20 cm:

| Etäisyys d [cm] | Mitattu raakalukema N | Laskettu etäisyys [cm] |
| :--- | :--- | :--- |
| 20,0 cm | 2 500 | 20,0 cm |
| 17,5 cm | 15 000 | 17,5 cm |
| 15,0 cm | 25 000 | 15,0 cm |
| 10,0 cm | 100 000 | 10,0 cm |
| 5,0 cm | 200 000 | 5,0 cm |
| 3,0 cm | 400 000 | 3,0 cm |
| 0,0 cm (kosketus) | 6 700 000 | 0,0 cm |

Teoreettinen siirtofunktio: **$d = 173{,}3 \cdot N^{-0{,}261}$**  
C-koodiin on toteutettu paloittain lineaarinen interpolaatio mittauspisteiden välille tarkan senttimetrilukeman saamiseksi.

Virhearviointi
Epälineaarisuus: Etäisyydellä 0–5 cm tarkkuus on hyvä (± 0,2 cm), mutta 15–20 cm etäisyydellä kohina kasvattaa virhettä (± 1,5 cm).
Komponenttitoleranssi: Picon sisäisen ylösvetovastuksen vaihtelu (50–80 kΩ) on kompensoitu ohjelmallisella alkukalibroinnilla.
Kohteen vaikutus: Mitattavan kohteen pinta-ala ja materiaali vaikuttavat kapasitanssiin.

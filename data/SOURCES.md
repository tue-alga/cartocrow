# Sources

The europe.ipe file is based on data from [naturalearthdata.com](https://www.naturalearthdata.com/).

The file example_isolines.ipe contains isolines generated from [The Reference Elevation Model of Antarctica (REMA)](https://www.pgc.umn.edu/data/rema/).
These DEMs were provided by the Byrd Polar and Climate Research Center and the Polar Geospatial Center under NSF-OPP awards 1043681, 1542736, 1543501, 1559691, 1810976, and 2129685.
See the end of the file for the relevant bibliography [3, 4].

The file nyc.txt contains the locations of hotels, subway entrances, and medical clinics (green) in lower Manhattan.
It is a common benchmark dataset that originates from the paper that introduced Bubble Sets [1].
The point locations were manually traced from the image in the paper.

The file diseasome.txt contains vertices of an embedded graph of disorders, from the human
disease network constructed by Goh et al. [2]. The dataset consists of 516 vertices (disorders) of
twenty-one disorder classes. We use the graph layout of a poster by Bastian and Heymann archived at 
https://web.archive.org/web/20121116145141/http://diseasome.eu/data/diseasome_poster.pdf.

The files gemeenten-2022_5000vtcs.ipe and dutch_selected.gpkg contain generalized municipalities of The Netherlands,
generated from the [Wijk- en buurtkaart 2022](https://www.cbs.nl/nl-nl/dossier/nederland-regionaal/geografische-data/wijk-en-buurtkaart-2022); © Kadaster / Centraal Bureau voor de Statistiek, 2024.
Furthermore, dutch_selected.gpkg contains six statistical data attributes present in the same Wijk- en buurtkaart.
The file apotheek_gemiddelde_afstand_in_km.csv is one of those six statistical data attributes in CSV format.
File hessen_selected.gpkg contains generalized municipalities of Hessen, Germany 
(https://sgx.geodatenzentrum.de/web_public/Datenquellen_vg_nuts.pdf; © BKG (2024) dl-de/by-2-0), 
and six statistical data attributes (https://statistik.hessen.de/; Hessische Gemeindestatistik, 2024).

## References
[1] C. Collins, G. Penn, and S. Carpendale. Bubble Sets: Revealing set relations with isocontours over existing visualizations.
IEEE Transactions on Visualization and Computer Graphics, 15(6):1009–1016, 2009. 
doi: https://doi.org/10.1109/TVCG.2009.122

[2] K.-I. Goh, M. E. Cusick, D. Valle, B. Childs, M. Vidal, and A.-L. Barabási.
The human disease network. Proceedings of the National Academy of
Sciences, 104(21):8685–8690, 2007. doi: https://doi.org/10.1073/pnas.0701361104

[3] Howat, Ian; Porter, Claire; Noh, Myoung-Jon; Husby, Erik; Khuvis, Samuel; Danish, Evan; 
Tomko, Karen; Gardiner, Judith; Negrete, Adelaide; Yadav, Bidhyananda; Klassen, James; 
Kelleher, Cole; Cloutier, Michael; Bakker, Jesse; Enos, Jeremy; Arnold, Galen; Bauer, Greg; 
Morin, Paul, 2022, "The Reference Elevation Model of Antarctica - Mosaics, Version 2", 
https://doi.org/10.7910/DVN/EBW8UC, Harvard Dataverse, V1 

[4] Howat, I. M., Porter, C., Smith, B. E., Noh, M. J., & Morin, P. (2019). The reference elevation model of Antarctica. 
The Cryosphere, 13(2), 665-674.

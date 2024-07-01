WITH MaxCommentLengths AS (
    SELECT
        work_type.id,
        work_type.name,
        MAX(LENGTH(work.comment)) AS max_length
    FROM
        work
    JOIN work_type ON work_type.id = work.type
    WHERE
        LENGTH(comment) > 0
    GROUP BY
        work_type.id,work_type.name
),
LongestComment AS (
    SELECT work_type.name AS work_type_name,
        w.name,
        LENGTH(w.comment) AS comment_length,
        w.comment
    FROM
        work w
    JOIN work_type ON work_type.id = w.type
    JOIN MaxCommentLengths mcl ON
        w.type = mcl.id AND LENGTH(w.comment) = mcl.max_length
    WHERE
        LENGTH(w.comment) > 0
)
SELECT
    work_type_name,
    name,
    comment_length,
    comment
FROM
    LongestComment
ORDER BY
    work_type_name ASC;

--.read q4_devil_in_the_details.sqlite.sql
--result
sqlite> .read q4_devil_in_the_details.sqlite.sql
Aria|Phi-Phi: La gamine charmante|92|arrangement by Jean Poiret and Dominique Tirmont for the show "II était une fois l'opérette"
Audio drama|The Adventures of Tom Bombadil|104|Brian Sibley drama for BBC Radio. Based not on the poem, but on material from The Fellowship of the Ring
Ballet|Les Mariés de la tour Eiffel|85|Ballet in One Act by members of the Groupe des Six from the spectacle by Jean Cocteau
Cantata|Jauchzet dem Herrn, alle Welt TWV 7:20|89|One of the 5 versions by Telemann: Cantata for Bass, Trumpet, Oboe, strings and continuo.
Concerto|Concerto in C major, RV 558, “con molti stromenti”|108|2 violins in tromba marina, 2 recorders, 2 trumpets, 2 mandolines, 2 chalumeaux, 2 theorboes, cello, strings
Incidental music|Rabelais|57|musique de scène pour le spectacle de Jean-Louis Barrault
Madrigal|As Fair as Morn, as Fresh as May|49|The Second Set of Madrigals for 3-6 voices, no. 5
Mass|Officium Defunctorum (1605)|104|With chants researched and edited from 16th- and 17th-century Spanish sources by Luis Lozano Virumbrales
Motet|Leçons de ténèbres du vendredy saint: Première leçon|63|De lamentatione Jeremiae prophetae. Heth. Misericordiae Domini.
Musical|John Paul Jones|72|A musical. Not to be confused with the orchestral works derived from it.
Opera|Prince Igor|76|opera by Borodin, most contemporary performances shouldn't link to this work
Operetta|I Have Been in Love Before|53|English version of "Gern hab' ich die Frau'n geküsst"
Oratorio|Markus-Passion, BWV 247|54|Reconstructed by Dietmar Hellmann and Andreas Glöckner
Overture|Preludio in si bemolle minore|140|1910, Ed. F. Bongiovanni - sopra un Corale di Bach "Ho sperato in te, o signore" ("In dich hab' ich gehoffet, Herr")(Cantata No. 52, finale)
Partita|Embroidery digitizing|143|Embroidery digitizing | Flat rate digitizing | Vector redraw | free embroidery designs pes, Machine embroidery designs, free embroidery designs
Play|Belicha|59|From a document in the British Library in London : Ms.29987
Poem|Ночное|109|«Если б мои не болели мозги, я бы заснуть не прочь. Рад, что в окошке не видно ни зги — ночь, чёрная ночь!..»
Poem|О собаках|109|«Не могу я видеть без грусти ежедневных собачьих драк, в этом маленьком захолустье поразительно много собак…»
Prose|The Man and the Birds|72|a Christmas story most notably narrated by radio broadcaster Paul Harvey
Quartet|The Irish Session Suite: Four Reels (4th Movement)|72|Paddy Fahy’s Reel / Beare Island Reel / Dowd’s Favourite / Boys of Malin
Sonata|Sonata for Wind Quartet no. 6 in F major|74|arranged for Wind Quartet by F. Berr from an Andante e Tema con variazioni
Song|Ragam Thanam Pallavi|180|Raga: Ragamalika(Ranjani, Manoranjani, Janaranjani, Sriranjani), Tala: Tisra Jampa (4 Kalai), Pallavi: Ranjani Niranjani Bhakta Manoranjani Rasika Janaranjani Varam Arul Sriranjani
Song-cycle|Sonnets From the Portuguese|93|composed by Libby Larsen, set to Elizabeth Barrett Browning's ca. 1845 work of the same title
Soundtrack|Empath Finale|115|contains "Theme From Star Trek (TV Series)" by Alexander Courage & Gene Roddenberry and "Mr. Spock" by Gerald Fried
Suite|Flatpyramid|180|We offer an extensive 3D model library that includes thousands of products spread across several categories (e.g. vehicles, architecture, characters, electronics, etc) and growing.
Symphonic poem|Kullervo, op. 7|53|for mezzo-soprano, baritone, male choir and orchestra
Symphony|Sinfonia concertante no. 11 in A major (KaiS Sc11 ICS 33)|147|Simphonie Concertante a plusieurs instruments; Sinfonia Concertante à Violino, Violoncello è Viola Conc:te - Violini Clarinetti Corni Viola e Basso
Zarzuela|Mirentxu|17|original zarzuela
Zarzuela|Marina|17|original zarzuela
Étude|Go From My Window|49|Fitzwilliam virginal book, nos. 9 (I) and 42 (II)

Run Time: real 0.239 user 0.169450 sys 0.069753
sqlite> 
Craftus Speicherformat

Weltenordner:

saves
|--/Weltenname
    |--/level.mpack
    |--/superchunks
        |--/superchunk-x-y-index.mpack
        |--/superchunk-x-y-data.dat

level.mpack
|--/string  name:   Weltenname
|--/list    players # Wird erstmal nur ein Element haben
    |--/float   x, y, z:    Position
    |--/float   yaw, pitch: Ausrichtung vom Spieler
    |--/bool    flying:     Ob der Spieler fliegt


Jeder Superchunk umfasst 128x128 Blöcke => 8 * 8 Chunks
Sämtliche kompressierten Daten sind zlib kompressiert!

superchunk-x-y-index.mpack
|--/list    chunkIndices:    Wo sich die Chunks befinden
    |--/int     position:   Position(in 4kb Sektoren) an der sich der Chunk befindet
    |--/byte    size:       Größe(in 4kb Sektoren)
    |--/int     actualSize: Die tatsächliche Größe
superchunk-x-y-data.mpack
Auf je 4kb Sektoren sind die Chunkdaten verteilt, wie in superchunk-x-y-index.mpack beschrieben
Chunk
|--/list    cluster:    Cluster des Chunks
    |--/bin blocks: Blöcke, im Chunk
|--/int     genProgress:    Fortschritt in der Weltgenerierung
|--/int     revision:       Version vom Chunk
|--/

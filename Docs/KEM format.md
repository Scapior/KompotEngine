KompotEngine model file - is binary a 3D model geometry data format.

Current KompotEngine Model format version is **1**.

File structure:

| HEADER <br />3 byte | CONSTANT<br />7 bytes | VERSION<br />1 byte | RESERVED<br />5 bytes | BLOCK 1 | BLOCK 2 | BLOCK N |
| ------------------- | --------------------- | ------------------- | --------------------- | ------- | ------- | ------- |
| 0x4b454d            | 0x494c5518112015      | Format file version | NULL                  | ...     | ...     | ...     |

Block structure:

| BLOCK  TYPE<br />1 byte | FLAGS<br />1 byte | RESERVED<br />2 byte | SIZE<br />4 byte | DATA           |
| ----------------------- | ----------------- | -------------------- | ---------------- | -------------- |
| Enum value              | Bit flags value   | NULL                 | uint32_t value   | Array of bytes |

**SIZE** is always contained the size of the DATA segment.

**BLOCK**s types:

* 0x01 - **Vertices**

  **FLAGS** field is currently reserved for future use.

  **DATA** is sequence of arrays [x, y, z] coordinates, where each of them present as *float*:

  <pre>
  point 0 x | point 0 y | point 0 z | point 1 x | ...
  </pre>

  Stored in **DATA** points count equals to *SIZE / ( 3 x 4)* .

* 0x02 - **Normals**

  **FLAGS** field is currently reserved for future use.

  **DATA** is a sequence of the [x, y, z] values, where each value present as a *float*.

  <pre>
  point 0 x | point 0 y | point 0 z | point 1 x | ...
  </pre>

  Stored in **DATA** normals values count equals to *SIZE / ( 3 x 4)* .

* 0x03 - **UV**

  **FLAGS** field is currently reserved for future use.

  **DATA** is a sequence of the [u, v] values of UV coordinates of a points, where each value present as a *float*.

  <pre>
  point 0 u | point 0 v | point 1 u | point 1 v | ...
  </pre>

  Stored in **DATA** normals values count equals to *SIZE / ( 2 x 4)* .

* 0x04 - **Mesh**

  **FLAGS** field is currently reserved for future use.

  **DATA** contains the sequence of an array of the three vertices indices for each triangle of a mesh. Vertex indices numeration start from zero, each index stored as uint32_t value.

  For example, the next block

  <pre>
  00 00 00 0<b>0</b>|00 00 00 0<b>1</b>|00 00 00 0<b>2</b>|00 00 00 0<b>1</b>|00 00 00 0<b>3</b>|00 00 00 0<b>2</b>
  </pre>

  contains two triangles:


<pre>
 0 *-----* 1
   |  /  |
 2 *-----* 3
</pre>

Blocks of equals types can be a few in file. In these case their index numeration will continue. For example:

Points Block 1 | Normals Block 2 | Points Block 2 | ...

In this case, if Points Block 1 contained N points with last point index N-1, a Points Block 2 first point will have index N.

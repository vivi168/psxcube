# PSX Cube

3D test on PSX

## Convert image

Save as 256 colors BMP

then

```
bmp2tim -org 640 0 -plt 0 480 -v CUBE.BMP
```

## compile

```
psymake
```

## Make CD

```
mkpsxiso -y mkpsxiso.xml
```

import gzip
import os
import datetime
Import("env")

files = [
  'data_embed/index.html',
  'data_embed/script.js',
]

for src in files:
  out = src + ".gz"
  
  
  with open(src, 'rb') as f:
    content = f.read()
      
  with open(out, 'wb') as f:
    f.write(gzip.compress(content, compresslevel=9))
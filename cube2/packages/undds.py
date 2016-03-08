import os, sys, subprocess

for f in sys.argv[1:]:
  print 'file:', f
  jpg = f.replace('.dds', '.jpg')
  png = f.replace('.dds', '.png')
  if not os.path.exists(jpg) and not os.path.exists(png):
    print '...create jpg'
    subprocess.check_call(['convert', f, jpg])


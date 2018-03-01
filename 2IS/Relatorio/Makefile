BASENAME=relatorio
TEXNAME=$(BASENAME).tex

all:
	pdflatex -shell-escape $(TEXNAME)
	pdflatex -shell-escape $(TEXNAME)
	pdflatex -shell-escape $(TEXNAME)
	open -a Preview $(BASENAME).pdf


clean:
	rm -f $(BASENAME).aux
	rm -f $(BASENAME).idx
	rm -f $(BASENAME).lof
	rm -f $(BASENAME).log
	rm -f $(BASENAME).lol
	rm -f $(BASENAME).lot
	rm -f $(BASENAME).toc
	rm -f *~
	rm -f $(BASENAME).bbl
	rm -f $(BASENAME).blg
	rm -f $(BASENAME).brf

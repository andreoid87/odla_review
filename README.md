## Setup GIT
Il tool per sviluppatori è utilizzato per cercare il numero di versione e visualizzarlo nella schermata “about” di ODLA.
* Andare su https://git-scm.com/downloads
* Cliccare sull’icona a forma di schermo (vedi immagine sotto)
* Una volta avviato l’installer continuare l’installazione lasciando tutte le impostazioni di default, fino alla fine.  
![Schermata pagina github](../master/readme_imgs/git_download.png)

## Setup QT 
Installare la versione “open source” che ovviamente è riservata a scopi non commerciali. _TODO_: valutare passaggio alla versione a pagamento.

* Andare su https://www.qt.io/download
* Scorrere la pagina fino a “Download for open source users”
* Cliccare su “Go open source” (tasto verde)
* Nella nuova pagina scorrere fino in fondo dov’è presente il pulsante verde “Download the Qt Online Installer”
* Di nuovo scorrere la pagina fino in fondo e cliccare su Download.
* Avviare l’installer, al primo passaggio vi verrà chiesto entrare nel vostro account QT o di crearne uno, createlo: la procedura guidata è semplice.
* Andate avanti accettando l’informativa, e facendogli scaricare il repository
* Nella schermata “seleziona i componenti” scegliere (vedi immagine sotto):
    * Qt 5.12.2 -> MinGw 7.3.0 32-bit  
    ![Kit QT](../master/readme_imgs/kit_qt.png)
    * Developer & Designer tools -> Qt Creator 4.x.x CDB Debugger Support
    * Developer & Designer tools -> Debugging Tools for Windows
    * Developer & Designer tools -> MinGw 7.3.0
    * Developer & Designer tools -> CMAKE 3.17.1 32-bit  
    ![Tools QT](../master/readme_imgs/tools_qt.png)

## Importare il progetto ODLA su QT
* Aprire QT Creator
* Scegliere New (accanto a Project) 
* Dal tab "import project" scegliere git Clone e cliccare su "Choose..."
* Compilare i dati della finestra di dialogo come da immagine  
![Schermata compilazione git in QT](../master/readme_imgs/git_import_qt.png)
* Inserire le credenziali quando richiesto ed infine configurare il progetto scegliendo il kit MinGw (vedi immagine sotto)  
![Schermata installazione QT](../master/readme_imgs/qt_select_kit.png)

# Compilazione su MAC
È necessario installare la libreria libusb, per fare questo bisogna dare i seguenti comandi da terminale:

```console
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
sudo chown -R $(whoami)/usr/local/Cellar
sudo chown -R $(whoami)/usr/local/Homebrew
sudo chown -R $(whoami)/usr/local/lib/python3.7/site-packages
sudo chown -R $(whoami)/usr/local/share/zsh
sudo chown -R $(whoami)/usr/local/share/zsh/site-functions
sudo chown -R $(whoami)/usr/local/var/homebrew/locks
brew install libusb
```


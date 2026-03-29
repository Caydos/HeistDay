@echo off
set UNREAL_EXE="C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT_FILE="C:\Users\Etudiant1\Desktop\HeistDay\FinalGame.uproject"

echo Lancement du SERVEUR DEDIE EOS...

:: Utilisation de 'start' pour lancer le processus et libérer la console
start "" %UNREAL_EXE% %PROJECT_FILE% -server -log -EpicApp=ServerArtifact

exit
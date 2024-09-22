# Flash

Skorzystaj z VSCode + PlatformIO. Przy podłączaniu urządzenia kliknij przycisk BOOT i następnie podłącz zasilanie lub przytrzymaj przycisk BOOT i wciśnij obok przycisk RESET

# Uruchomienie

Włączenie urządzenia jest sygnalizowane piknięciem. W przypadku braku połączenia z WiFi jest uruchamiany WiFi AP i jest to sygnalizowane drugim piknięciem. Domyślne WiFi AP to `BalloonMeterAP` bez hasła. Po uruchomieniu urządzenia aktywuje pompkę aby zrobić offset, potrwa to 5s - dzięki takiemu rozwiązaniu podczas pompowania będziemy wiedzieli jakie jest ciśnienie i nie będzie trzeba wyłączać pompki aby sprawdzić ciśnienie.

# Kalibracja

## Krok 1

Wstępna kalibracja to kalibracja mnożnika. Jest to zmienna `multiplier` w pliku `data_embed/script.js`. Jeśli ciśnienie na odczycie realnym (z równoległego manometra) jest większe to zwiększaj zmienną `multiplier`, w przeciwnym razie ją zmniejszaj.

## Krok 2

Kolejna kalibracja jest liniowa. Zmienne `mapInMax` i `mapOutMax` w pliku `data_embed/script.js`. Napompuj balon do docelowego ciśnienia a nastepnie w zmienną `mapInMax` wstaw wartość `C: ` z strony a w `mapOutMax` wstaw aktualne ciśnienie z zew. manometru. Interesuje nas aby dokładność była najwyższa przy ciśnieniu docelowym.

Krok 2 może wcale nie być potrzebny jeśli nie planujemy pompować zbyt wysokiego ciśnienia.

W obu przypadkach na stronie www możesz dynamicznie testować zmienne ustawiając je w konsoli przeglądarki np. `multiplier=0.00131`, następnie po dokładnym przetestowaniu można zrobić flash z pewnymi wartościami.

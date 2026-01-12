struct ParsedLineData
    element_type : ElementType [RAMP|WORKER|STOREHOUSE|LINK]
    parameters : string -> string>  # mapowanie "ID -> wartość"


parse_line(line : string)
    rozłóż linię na ID określający typ elementu oraz tokeny "klucz=wartość"
    każdy token rozłóż na parę (klucz, wartość)
    na podstawie ID oraz par (klucz, wartość) zwróć odpowiedni obiekt typu ParsedLineData


load_factory_structure(is: std::istream&)
    utwórz (pusty) obiekt typu Factory

    dla każdej linii w pliku
        jeśli linia pusta lub rozpoczyna się od znaku komentarza - przejdź do kolejnej linii
        dokonaj parsowania linii
        w zależności od typu elementu - wykorzystaj pary (klucz, wartość) do poprawnego:
         * zainicjalizowania obiektu właściwego typu węzła i dodania go do obiektu fabryki, albo
         * utworzenia połączenia między zadanymi węzłami sieci

    zwróć wypełniony obiekt fabryki

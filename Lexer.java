import java.io.*; 
import java.util.*;
import java.lang.*;     //per la funzione endsWith()

//QUELLO CORRETTO

public class Lexer {

    public static int line = 1;
    private char peek = ' ';
    
    private void readch(BufferedReader br) {
        try {
            peek = (char) br.read();
        } catch (IOException exc) {
            peek = (char) -1; // ERROR
        }
    }

    public Token lexical_scan(BufferedReader br) {
        while (peek == ' ' || peek == '\t' || peek == '\n'  || peek == '\r') {  //gestione caratteri di separazione
            if (peek == '\n') line++;
            readch(br);
        }
        
        if(peek=='/'){      //gestione commenti
                readch(br);
                if(peek=='/'){
                    readch(br);
                    while(peek!='\n'){
                        readch(br);
                    }
                    readch(br);
                }
                else if(peek=='*'){
                    String comment="";
                    while(!comment.endsWith("*/")){
                        comment+=peek;
                        readch(br);
                    }
                    readch(br);
                }
                else
                    return Token.div;
        }
        
        switch (peek) {     //gestione caratteri speciali con pattern singolo
            case '!':
                peek = ' ';
                return Token.not;
            case '(':
                peek = ' ';
                return Token.lpt;
            case ')':
                peek = ' ';
                return Token.rpt;
            case '{':
                peek = ' ';
                return Token.lpg;
            case '}':
                peek = ' ';
                return Token.rpg;
            case '+':
                peek = ' ';
                return Token.plus;
            case '-':
                peek = ' ';
                return Token.minus;
            case '*':
                peek = ' ';
                return Token.mult;
            case ';':
                peek = ' ';
                return Token.semicolon;
            case '&':           //gestione token con pattern da 2 caratteri
                readch(br);
                if (peek == '&') {
                    peek = ' ';
                    return Word.and;
                } else {
                    System.err.println("Erroneous character"
                            + " after & : "  + peek );
                    return null;
                }
            case '|':
                readch(br);
                if (peek == '|') {
                    peek = ' ';
                    return Word.or;
                } else {
                    System.err.println("Erroneous character"
                            + " after | : "  + peek );
                    return null;
                }
            case '<':
                readch(br);
                if(peek=='='){
                    peek=' ';
                    return Word.le;
                }
                else if(peek=='>'){
                    peek=' ';
                    return Word.ne;
                } else return Word.lt;
            case '>':
                readch(br);
                if(peek=='='){
                    peek=' ';
                    return Word.ge;
                } else return Word.gt;
            case '=':
                readch(br);
                if (peek == '=') {
                    peek = ' ';
                    return Word.eq;
                } else {
                    System.err.println("Erroneous character"
                            + " after = : "  + peek );
                    return null;
                }
            case ':':
                readch(br);
                if (peek == '=') {
                    peek = ' ';
                    return Word.assign;
                } else {
                    System.err.println("Erroneous character"
                            + " after : : "  + peek );
                    return null;
                }
            case '_':    //identificatore che comincia con '_'
                String identifier="";
                boolean correct=false;
                while(Character.isDigit(peek)||Character.isLetter(peek)||peek=='_'){
                    identifier+=peek;
                    if(peek!='_')
                        correct=true;
                    readch(br);
                }
                if(correct)
                    return new Word(Tag.ID,identifier);
                else {
                    System.err.println("Erroneous identifier definition: "+ peek );
                    return null;
                }
            case (char)-1:
                return new Token(Tag.EOF);

            default:
                if (Character.isLetter(peek)) {     //Gestione parole chiave e identificatori che non cominciano con '_'
                    String word="";
                    boolean found=false;
                    while(Character.isLetter(peek) && !found){
                        word+=peek;
                        found=word.equals("case") ||
                              word.equals("when") ||
                              word.equals("then") ||
                              word.equals("else") ||
                              word.equals("while") ||
                              word.equals("do") ||
                              word.equals("print") ||
                              word.equals("read");
                        readch(br);
                    }
                    if(found){          //parola chiave
                        if(word.equals("case"))
                            return Word.casetok;
                        else if(word.equals("when"))
                            return Word.when;
                        else if(word.equals("then"))
                            return Word.then;
                        else if(word.equals("else"))
                            return Word.elsetok;
                        else if(word.equals("while"))
                            return Word.whiletok;
                        else if(word.equals("do"))
                            return Word.dotok;
                        else if(word.equals("print"))
                            return Word.print;
                        else
                            return Word.read;
                    } else {            //identificatore
                        while(Character.isDigit(peek)||Character.isLetter(peek)||peek=='_'){
                            word+=peek;
                            readch(br);
                        }
                        return new Word(Tag.ID,word);
                    }
                }
                else if (Character.isDigit(peek)) {       //numero
                    String number="";
                    while(Character.isDigit(peek)){
                        number+=peek;
                        readch(br);
                    }
                    return new NumberTok(Integer.parseInt(number));

                } else {        //inserimento errato
                        System.err.println("Erroneous character: " 
                                + peek );
                        return null;
                }
         }
    }
		
    public static void main(String[] args) {
        Lexer lex = new Lexer();
        String path = "Z:/Desktop/181123/test.txt"; // il percorso del file da leggere
        try {
            BufferedReader br = new BufferedReader(new FileReader(path));
            Token tok;
            do {
                tok = lex.lexical_scan(br);
                System.out.println("Scan: " + tok);
            } while (tok.tag != Tag.EOF);
            br.close();
        } catch (IOException e) {e.printStackTrace();}    
    }

}
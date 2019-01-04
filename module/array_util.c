//modulo per gestire l'array degli invitati
//si assume che l'array sia inizializzato con valori -1

//ancora da testare!

//se e' il caso integrarlo in un altro modulo

//restituisce la lunghezza dell'array
// -1 se e' NULL
int length(int *a){
    if(a){
        int i;
        for(i=0;i<ARRAY_LEN && a[i]!=-1;i++);
        return i;
    }
    else
        return -1;
}

//aggiunge un membro
// 0: successo, -1: array=NULL o raggiunta la massima lunghezza
int add_member(int *a, int mem){
    if(a && mem>=0){
        int i;
        if((i=length(a)) < ARRAY_LEN){
            a[i]=mem;
            return 0;
        }
        else
            return -1;
    }
    else
        return -1;
}

//rimuove un membro
// 0: successo, -1: il array=NULL o il membro non esiste
int rm_member(int *a, int mem){
    if(a && mem>=0){
        int i,
            found = FALSE,
            length = length(a);
        for(i=0;i<length;i++){
            if(a[i]==mem)
                found = TRUE;
            if(found)
                a[i]=a[i+1];
        }
        if(found)
            return 0;
        else
            return -1;
        
    }
    else
        return -1;
}
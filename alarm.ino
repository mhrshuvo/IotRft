void alarm(){

  int i = 0;
  do{
    i++;
    tone(buzz, 1000, 1500);
  } while(i<3);
  
}

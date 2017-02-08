int main()
{
int i=0;
int j=0;
char* pcVideoMemory = (char*) 0xB8000;
char* pcMessage = "MINT64 OS Boot";
char cTemp;

while(1)
{
cTemp = pcMessage[i];
if(cTemp == 0) break;
pcVideoMemory[j] = cTemp;
i+=1; // char*
j+=2; // memory
}
}

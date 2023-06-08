extern "C"
{

extern int main ();
extern void _test ();

int _kernel_entry()
{
    main ();
    
    return 0;
}


}
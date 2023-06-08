extern "C"
{

extern int main ();
extern void _test ();

int _kernel_entry()
{
    main ();

    _test ();
    
    return 0;
}


}
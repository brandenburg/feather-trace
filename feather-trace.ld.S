SECTIONS
{
    __event_table :
    {
        __start___event_table = .;
	*(__event_table)
	__stop___event_table = .;
    }
}
INSERT AFTER .text;

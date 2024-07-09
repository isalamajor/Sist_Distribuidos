struct user_operation {
    string username<256>; /* Máximo 256 caracteres para el nombre de usuario */
    string operation<13>;
    string filename<256>;
    string date_time<20>; /* Fecha y hora */
};

program OPERACIONES {
    version OPS_VERSION {
        void get_operation(struct user_operation) = 1;
    } = 1;
} = 2;

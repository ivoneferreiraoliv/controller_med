Projeto de Linguagem de Programação I
Ciência da Computação, Universidade Estadual da Paraíba

Autor(a): Ivone Maria Ferreira

Sistema de Controle de Medicamentos
Descrição do Projeto:
Este projeto foi desenvolvido como parte da disciplina de Linguagem de Programação I do curso de Ciências da Computação.
A ideia surgiu a partir de uma necessidade real de controle de estoque de medicamentos, permitindo um melhor gerenciamento e organização dos mesmos, além de assegurar que as dosagens sejam administradas nos horários corretos.

Funcionalidades:
Cadastro de Medicamentos

Permite aos usuários adicionar novos medicamentos ao sistema, incluindo informações como nome, dosagem, quantidade em estoque e data de validade.
Registro de Horários de Dosagem

Para cada medicamento cadastrado, os usuários podem registrar os horários em que devem ser administradas as dosagens, ajudando na organização e no cumprimento do tratamento.
Alertas de Reposição de Estoque

O sistema monitora o estoque de medicamentos e exibe alertas quando a quantidade de um medicamento atinge um nível crítico, indicando a necessidade de reposição.
Visualização e Edição de Medicamentos

Os usuários podem visualizar todos os medicamentos cadastrados, editar informações existentes e remover medicamentos que não são mais necessários.
Relatórios de Utilização

O sistema fornece relatórios detalhados sobre a utilização dos medicamentos, incluindo histórico de dosagens, datas de reposição de estoque e estatísticas de consumo.

Tecnologias Utilizadas:
Linguagem de Programação: C
Banco de Dados: SQLite
Bibliotecas: stdio.h, stdlib.h, string.h, sqlite3.h
Estrutura do Código
O projeto está estruturado em várias funções principais para manusear a lógica de cadastro, atualização e gerenciamento dos medicamentos e horários de dosagem.

Funções Principais:
abrirBancoDeDados()

Abre a conexão com o banco de dados SQLite.
fecharBancoDeDados()

Fecha a conexão com o banco de dados SQLite.
criarTabelaMedicamentos()

Cria a tabela de medicamentos no banco de dados, se ainda não existir.
criarTabelaHorariosDosagem()

Cria a tabela de horários de dosagem no banco de dados, se ainda não existir.
cadastrarMedicamento()

Solicita ao usuário os dados do medicamento e os insere na tabela de medicamentos.
registrarHorarioDosagem()

Solicita ao usuário os horários de dosagem e os insere na tabela de horários de dosagem.

Exemplo de Uso:
No main(), a conexão com o banco de dados é aberta, as tabelas são criadas e um exemplo de cadastro de medicamento é executado.

int main() {
    abrirBancoDeDados();
    criarTabelaMedicamentos();
    criarTabelaHorariosDosagem();

    cadastrarMedicamento();

    fecharBancoDeDados();
    return 0;
}

Contribuição:
No momento, não estamos aceitando contribuições, pois o projeto ainda está em desenvolvimento.

Licença:
Este projeto é de uso livre para fins educacionais e de desenvolvimento. Sinta-se à vontade para utilizar e estudar o sistema conforme necessário.

Este projeto é um exemplo prático de como a programação pode resolver problemas reais e ajudar na organização e eficiência de tarefas cotidianas. Agradecemos a todos os colaboradores e esperamos que este sistema seja útil e inspirador para futuras inovações.

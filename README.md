# FlowMon

**FlowMon é um editor de workflow visual, construído para a web. Crie, execute e monitore tarefas e scripts de shell em múltiplos dispositivos locais ou remotos com uma interface de arrastar e soltar.**

[![React](https://img.shields.io/badge/React-18-blue?logo=react)](https://react.dev/)
[![Node.js](https://img.shields.io/badge/Node.js-20-green?logo=nodedotjs)](https://nodejs.org/)
[![React Flow](https://img.shields.io/badge/React_Flow-11-orange)](https://reactflow.dev/)

## Sobre o Projeto

FlowMon transforma a complexidade da automação de sistemas e da criação de scripts em uma experiência visual e interativa. A plataforma permite que usuários, desde administradores de sistemas a desenvolvedores, desenhem fluxos de execução complexos conectando nós funcionais em uma tela. O sistema então gera automaticamente os scripts de shell correspondentes, que podem ser executados e monitorados em tempo real diretamente da interface.

Este projeto foi desenhado para ser uma solução completa, cobrindo desde a autenticação de usuários e gerenciamento de projetos até a execução remota segura via SSH e a visualização de métricas de performance.

<img width="1596" height="806" alt="image" src="https://github.com/user-attachments/assets/989be006-9cfb-4348-b4ee-08918a5529bc" />

## Pré-requisitos

- **Sistema Operacional**: Linux (recomendado Ubuntu 24.04.2 LTS ou Linux Mint 22.1)
- **Node.js**: v22.17.1
- **npm**: v11.4.2
- **Docker**: v28.3.2

## Instalação e Configuração

### 1. Clone o repositório

```bash
git clone https://github.com/eduardolima1994/flow.git
cd flow
```

### 2. Configuração do Backend

#### Instale as dependências
```bash
cd backend
npm i
```

#### Configure o arquivo .env
Crie um arquivo `.env` no diretório `backend` com as seguintes configurações:

```env
JWT_SECRET=your_secret_key_here
DB_HOST=localhost 
DB_USER=flowmon_user
DB_PASSWORD=flowmon_password
DB_NAME=flowmon_db
DB_PORT=5432
```

#### Inicialize o banco de dados
```bash
# Inicie o contêiner PostgreSQL
docker compose up -d db

# Configure o Sequelize (se necessário)
npx sequelize-cli init
```

#### Execute o backend
```bash
npm run dev
```

### 3. Configuração do Frontend

#### Instale as dependências
```bash
cd ../frontend
npm i
```

#### Execute o frontend
```bash
npm run dev
```

## Acesso à Aplicação

1. Acesse a aplicação em: [http://localhost:5173/](http://localhost:5173/)
2. Navegue até `/login`
3. Faça login com um dos usuários padrão:

| Usuário | Senha |
|---------|-------|
| `admin` | `admin_password` |
| `user` | `user_password` |

## Configuração Adicional

### Habilitando o Registro de Novos Usuários

Por padrão, a rota de registro (`/register`) requer autenticação no arquivo `server.js`:

```javascript
app.post("/api/register", authenticateToken, async (req, res) => {...}
```

Para permitir registro público de novos usuários, remova o middleware `authenticateToken`:

```javascript
app.post("/api/register", async (req, res) => {...}
```

## Principais Funcionalidades

- **Editor Visual de Arrastar e Soltar**: Crie workflows complexos de forma intuitiva usando uma vasta biblioteca de nós pré-definidos (`CPU`, `Memória`, `Disco`, `Ping`, `Serviços`, e mais).
- **Geração de Scripts em Tempo Real**: Converta seus diagramas visuais em scripts de shell (`.sh`) prontos para execução com um único clique.
- **Execução Remota (SSH)**: Configure e execute tarefas em múltiplos dispositivos remotos de forma segura, com suporte a autenticação por senha.
- **Terminal e Monitoramento Live**: Acompanhe a saída `stdout`/`stderr` de suas execuções e visualize gráficos de performance (CPU, Memória, etc.) em tempo real.
- **Gerenciamento de Projetos**: Sistema completo de autenticação de usuários para salvar, carregar, listar e deletar projetos, garantindo que seu trabalho esteja sempre seguro e organizado.
- **Funcionalidades Avançadas de Workflow**:
  - **Clonagem de Dispositivos**: Clone um dispositivo e todo o seu fluxo de execução associado.
  - **Clonagem em Massa**: Importe um arquivo `.csv` para clonar um fluxo para dezenas ou centenas de dispositivos de uma só vez.
  - **Colapsar/Expandir Fluxos**: Mantenha a tela organizada ocultando partes do fluxo que não estão em foco.
- **Importação e Exportação**:
  - **Projetos**: Exporte e importe seus workflows completos no formato `.json` para backup ou compartilhamento.
  - **Diagramas UML**: Exporte uma representação visual do seu fluxo como uma imagem `.png` usando PlantUML.

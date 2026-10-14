import { HeaderProps } from "../types";

function Header(props: HeaderProps){

    return (
        <header>
            <h1>TO-DO List</h1>
            <p>Gerencie suas tarefas.</p>

            <input id="task-input" type="text" placeholder="O que você planeja fazer?" />
            <button onClick={props.handleNewTaskClick}>
                <i> + </i>
            </button>

            <section>
                <span>Pendentes: {props.pendingTasks}</span>
                <span>Concluídas: {props.completedTasks}</span>
            </section>

        </header>
    )
}

export default Header;
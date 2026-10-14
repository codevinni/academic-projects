import { HeaderProps } from "../types";

function Header(props: HeaderProps){
    return (
        <header className="header">
            <div className="header-brand">
                <h1 className="header-title"><span className="title-accent">TO-DO</span> LIST</h1>
            </div>
            <p className="header-subtitle">Gerencie suas tarefas diárias com foco</p>

            <div className="task-input-group">
                <div className="input-inner-wrapper">
                    <svg className="input-search-icon" width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                        <circle cx="11" cy="11" r="8"></circle>
                        <line x1="21" y1="21" x2="16.65" y2="16.65"></line>
                    </svg>
                    <input 
                        id="task-input" 
                        type="text" 
                        placeholder="O que você planeja fazer hoje?" 
                        autoComplete="off"
                        onKeyDown={(e) => {
                            if (e.key === "Enter") {
                                props.handleNewTaskClick();
                            }
                        }}
                    />
                </div>
                <button 
                    type="button"
                    className="btn-add-task" 
                    onClick={props.handleNewTaskClick}
                    aria-label="Adicionar nova tarefa"
                >
                    <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.6" strokeLinecap="round" strokeLinejoin="round">
                        <line x1="12" y1="5" x2="12" y2="19"></line>
                        <line x1="5" y1="12" x2="19" y2="12"></line>
                    </svg>
                </button>
            </div>

            <section className="stats-display-panel" aria-label="Resumo de tarefas">
                <div className="stat-item stat-item--pending">
                    <span className="stat-item-icon">
                        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.4" strokeLinecap="round" strokeLinejoin="round">
                            <circle cx="12" cy="12" r="10"></circle>
                            <polyline points="12 6 12 12 16 14"></polyline>
                        </svg>
                    </span>
                    <span className="stat-label">Pendentes:</span>
                    <span className="stat-value">{props.pendingTasks}</span>
                </div>

                <div className="stat-divider" aria-hidden="true"></div>

                <div className="stat-item stat-item--completed">
                    <span className="stat-item-icon">
                        <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.4" strokeLinecap="round" strokeLinejoin="round">
                            <path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"></path>
                            <polyline points="22 4 12 14.01 9 11.01"></polyline>
                        </svg>
                    </span>
                    <span className="stat-label">Concluídas:</span>
                    <span className="stat-value">{props.completedTasks}</span>
                </div>
            </section>
        </header>
    )
}

export default Header;
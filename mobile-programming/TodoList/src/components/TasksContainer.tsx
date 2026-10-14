import { useState, useEffect } from "react";
import { TasksContainerProps } from "../types";

function TasksContainer({ tasks, currentFilter, handleTaskClick }: TasksContainerProps){

    const [exitingTaskIds, setExitingTaskIds] = useState<number[]>([]);

    // Limpa tarefas em saída caso o filtro seja alterado
    useEffect(() => {
        setExitingTaskIds([]);
    }, [currentFilter]);

    const handleAnimatedTaskClick = (id: number) => {
        // Se estiver no filtro 'all', a tarefa não sai da lista
        if (currentFilter === 'all') {
            handleTaskClick(id);
            return;
        }

        // Se estiver em 'pending' ou 'completed', anima o card sumindo antes de atualizar a lista
        if (!exitingTaskIds.includes(id)) {
            setExitingTaskIds(prev => [...prev, id]);
            setTimeout(() => {
                handleTaskClick(id);
                setExitingTaskIds(prev => prev.filter(taskId => taskId !== id));
            }, 320);
        }
    };

    const filteredTasks = tasks.filter((task) => {
        if (currentFilter === 'completed') 
            return task.completed;
        if (currentFilter === 'pending') 
            return !task.completed;
        return true;
    });

    const getEmptyMessage = () => {
        if (currentFilter === 'completed') return "Nenhuma tarefa concluída ainda.";
        if (currentFilter === 'pending') return "Nenhuma tarefa pendente! Parabéns.";
        return "Nenhuma tarefa adicionada. Comece criando uma acima!";
    };

    return (
        <section className="tasks-section" aria-label="Lista de tarefas">
            <div className="tasks-list" key={currentFilter}>
                {filteredTasks.length === 0 ? (
                    <div className="tasks-empty-state">
                        <div className="empty-icon-box">
                            <svg width="28" height="28" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.8" strokeLinecap="round" strokeLinejoin="round">
                                <circle cx="12" cy="12" r="10"></circle>
                                <line x1="8" y1="12" x2="16" y2="12"></line>
                            </svg>
                        </div>
                        <p className="empty-title">Lista vazia</p>
                        <p className="empty-subtitle">{getEmptyMessage()}</p>
                    </div>
                ) : (
                    filteredTasks.map((task, index) => {
                        const isExiting = exitingTaskIds.includes(task.id);
                        return (
                            <div 
                                key={`${currentFilter}-${task.id}`} 
                                onClick={() => handleAnimatedTaskClick(task.id)}
                                onKeyDown={(e) => {
                                    if (e.key === "Enter" || e.key === " ") {
                                        e.preventDefault();
                                        handleAnimatedTaskClick(task.id);
                                    }
                                }}
                                tabIndex={0}
                                role="checkbox"
                                aria-checked={task.completed}
                                className={`task-card ${task.completed ? 'task-card--completed' : 'task-card--pending'} ${isExiting ? 'task-card--exiting' : ''}`}
                                style={{ animationDelay: `${Math.min(index * 0.05, 0.3)}s` }}
                            >
                            <div className={`task-checkbox-circle ${task.completed ? 'task-checkbox-circle--checked' : ''}`}>
                                <svg 
                                    className="task-check-icon" 
                                    width="14" 
                                    height="14" 
                                    viewBox="0 0 24 24" 
                                    fill="none" 
                                    stroke="currentColor" 
                                    strokeWidth="3.2" 
                                    strokeLinecap="round" 
                                    strokeLinejoin="round"
                                >
                                    <polyline points="20 6 9 17 4 12"></polyline>
                                </svg>
                            </div>
                            <span className="task-description">{task.description}</span>
                        </div>
                        );
                    })
                )}
            </div>
        </section>
    )
}

export default TasksContainer;
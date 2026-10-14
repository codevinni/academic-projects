import { TasksContainerProps } from "../types";

function TasksContainer({ tasks, currentFilter, handleTaskClick }: TasksContainerProps){

    const filteredTasks = tasks.filter((task) => {
        if (currentFilter === 'completed') 
            return task.completed;
        if (currentFilter === 'pending') 
            return !task.completed;
        return true;
    });

    return (
        <>
            {filteredTasks.map(task => (
                <div key={task.id} onClick={() => handleTaskClick(task.id)}>
                    {task.completed ? 
                        <span>
                            <i> v - </i> 
                        </span>
                        :
                        <span>
                            <i> o - </i>
                        </span>
                    }
                    <span>{task.description}</span>
                </div>
            ))}
        </>
    )
}

export default TasksContainer;